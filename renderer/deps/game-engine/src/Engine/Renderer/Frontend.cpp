#include "Frontend.h"
#include "Backends/Vulkan/VKRenderer.h"
#include "../Log.h"
#include "../Config.h"
#include "../FileLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <imgui.h>
#include <SDL_mouse.h>

const std::string frameBufferVertexFile = "fbo.vert";
const std::string frameBufferFragFile = "fbo.frag";

const u32 MAX_UI_LAYER = 50;

RenderBackend* RenderFrontend::m_Backend = nullptr;

std::map<std::string, ModelTree> RenderFrontend::mLoadedModels;
std::map<std::string, Texture*> RenderFrontend::mLoadedTextures;
std::map<std::string, std::vector<Character>> RenderFrontend::mLoadedFonts;
std::map<std::string, Shader*> RenderFrontend::mLoadedShaders;

int RenderFrontend::mScreenX = 0;
int RenderFrontend::mScreenY = 0;

CameraComponent* RenderFrontend::mainCamera = nullptr;

std::vector<Drawable> RenderFrontend::mWorldToDraw;
std::vector<Drawable> RenderFrontend::mUIToDraw;

Shader* RenderFrontend::m_TextShader = nullptr;
Shader* RenderFrontend::m_SpriteShader = nullptr;
Model RenderFrontend::m_UIModel;

bool RenderFrontend::m_DrawUI = true;
Mat4 RenderFrontend::m_ShaderUserData = Mat4(0.0f);
Mat4 RenderFrontend::m_AspectMatrix = Mat4(1.0f);
DirectionalLightData RenderFrontend::m_DirectionalData = {Vec4(0.0f), Vec4(0.0f), Vec4(0.0f), Vec4(0.0f)};

static Mat4 AssimpMat4ToMat4(const aiMatrix4x4& aiMatrix) {
  return Mat4(aiMatrix.a1, aiMatrix.b1, aiMatrix.c1, aiMatrix.d1,
              aiMatrix.a2, aiMatrix.b2, aiMatrix.c2, aiMatrix.d2,
              aiMatrix.a3, aiMatrix.b3, aiMatrix.c3, aiMatrix.d3,
              aiMatrix.a4, aiMatrix.b4, aiMatrix.c4, aiMatrix.d4);
}

static void CopyNode(std::shared_ptr<Node>& dstNode, const aiNode* srcNode) {

  dstNode->mTransformMatrix = AssimpMat4ToMat4(srcNode->mTransformation);
  dstNode->mMeshIndices.resize(srcNode->mNumMeshes);

  for (u32 i = 0; i < srcNode->mNumMeshes; i++) {
    dstNode->mMeshIndices[i] = srcNode->mMeshes[i];
  }

  dstNode->mChildren.resize(srcNode->mNumChildren);
  for (u32 i = 0; i < srcNode->mNumChildren; i++) {
    std::shared_ptr<Node> child = std::make_shared<Node>();
    dstNode->mChildren[i] = child;
    CopyNode(child, srcNode->mChildren[i]);
  }
}

void RenderFrontend::Init() {
  m_Backend = new VKBackend;

  mScreenX = Config::GetOptionInt("Width");
  mScreenY = Config::GetOptionInt("Height");

  if (Config::OptionExists("DrawUI")) {
    m_DrawUI = Config::GetOptionInt("DrawUI") == 1;
  } else {
    m_DrawUI = true;
  }

  m_Backend->Init();

  m_Backend->WindowInit("Foveated Rendering", mScreenX, mScreenY);

  Model fboModel = LoadModel("models/fbo.obj").mMeshes[0];
  m_Backend->SetFrameBufferModel(fboModel);
  m_Backend->SetFrameBufferShader(LoadShader(frameBufferVertexFile, frameBufferFragFile, DRAW_STAGE::UI), DRAW_STAGE::UI);
  m_Backend->SetFrameBufferShader(LoadShader(frameBufferVertexFile, frameBufferFragFile, DRAW_STAGE::ASPECT), DRAW_STAGE::ASPECT);

  mainCamera = nullptr;

  m_TextShader = LoadShader("sprite.vert", "text.frag", DRAW_STAGE::UI);
  m_SpriteShader = LoadShader("sprite.vert", "sprite.frag", DRAW_STAGE::UI);
  m_UIModel = LoadModel("models/sprite.obj").mMeshes[0];

  //Setup matrix for correcting aspect ratio scaling in ui
  const IVec2 screenRes = GetScreenResolution();

  float aspectRatio = (float)(screenRes.x) / screenRes.y;
  Transform aspectTransform;
  aspectTransform.rotation = Vec3(0.0f);
  aspectTransform.position = Vec3(0.0f);
  aspectTransform.scale = Vec3(1 / aspectRatio, 1.0f, 1.0f);

  m_AspectMatrix = TransformToMat4(aspectTransform);
}

void RenderFrontend::Shutdown() {
  if (m_Backend != nullptr) {
    if (mLoadedTextures.size() > 0) {
      for (auto it = mLoadedTextures.begin(); it != mLoadedTextures.end(); it++) {
        DeleteTexture(it->second);
      }
    }
    if (mLoadedModels.size() > 0) {
      for (auto models : mLoadedModels) {
        for (auto model : models.second.mMeshes) {
          m_Backend->DeleteModel(model);
        }
      }
    }
    if (mLoadedShaders.size() > 0) {
      for (auto shader : mLoadedShaders) {
        m_Backend->DeleteShader(shader.second);
      }
    }

    if (mLoadedTextures.size() > 0) {
      for (auto texture : mLoadedTextures) {
        m_Backend->DeleteTexture(texture.second);
      }
    }

    if (mLoadedFonts.size() > 0) {
      for (auto font : mLoadedFonts) {
        for (auto character : font.second) {
          m_Backend->DeleteTexture(character.mTexture);
        }
      }
    }
    m_Backend->Shutdown();
    delete m_Backend;
  }
}

ModelTree RenderFrontend::LoadModel(const std::string &file) {
  auto path = FileLoader::GetRootPath();
  path.append(file);

  //Try to find an existing model first
  auto it = mLoadedModels.find(file);
  if (it != mLoadedModels.end()) {
    return mLoadedModels.at(file);
  }

  ModelTree modelTree;

  //Load meshes
  std::vector<Model> models;
  Assimp::Importer import;
  const aiScene *scene = import.ReadFile(path.generic_string(),
     aiProcess_Triangulate | aiProcess_GenNormals
    | aiProcess_FlipUVs | aiProcess_OptimizeMeshes 
    | aiProcess_FindInvalidData);

  if (scene == nullptr) {
    Log::LogFatal("COULD NOT FIND MODEL FILE: " + file);
    exit(1);
  }

  for (int meshNum = 0; meshNum < scene->mNumMeshes; meshNum++) {
    const aiMesh *mesh = scene->mMeshes[meshNum];
    aiColor3D color (1.0f, 1.0f, 1.0f);
    aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
    mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    Model model;
    //Copy all vertex positions/normals
    std::vector<Vertex> vertices(mesh->mNumVertices);
    for (size_t i = 0; i < vertices.size(); i++) {
      vertices[i].mPosition.x = mesh->mVertices[i].x;
      vertices[i].mPosition.y = mesh->mVertices[i].y;
      vertices[i].mPosition.z = mesh->mVertices[i].z;
      vertices[i].mNormal.x = mesh->mNormals[i].x;
      vertices[i].mNormal.y = mesh->mNormals[i].y;
      vertices[i].mNormal.z = mesh->mNormals[i].z;
      if (mesh->HasTextureCoords(0)) {
        vertices[i].mTexCoord.x = mesh->mTextureCoords[0][i].x;
        vertices[i].mTexCoord.y = mesh->mTextureCoords[0][i].y;
      } else {
        vertices[i].mTexCoord = { 0.0f, 0.0f };
      }

      if (mesh->HasVertexColors(0)) {
        vertices[i].mColor.r = mesh->mColors[0][i].r;
        vertices[i].mColor.g = mesh->mColors[0][i].g;
        vertices[i].mColor.b = mesh->mColors[0][i].b;
      } else {
        vertices[i].mColor.r = color.r;
        vertices[i].mColor.g = color.g;
        vertices[i].mColor.b = color.b;
      }
    }


    //Store indices
    std::vector<u32> indices(3 * mesh->mNumFaces);
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
      indices[3 * i] = mesh->mFaces[i].mIndices[0];
      indices[(3 * i) + 1] = mesh->mFaces[i].mIndices[1];
      indices[(3 * i) + 2] = mesh->mFaces[i].mIndices[2];
    }

    //Copy data to GPU
    model = m_Backend->LoadModel(vertices, indices);
    model.mNumFaces = mesh->mNumFaces;
    models.push_back(model);
  }

  modelTree.mMeshes = models;
  modelTree.mShader = nullptr;

  modelTree.mRoot = std::make_shared<Node>();
  //Copy root node
  std::shared_ptr<Node> root = modelTree.mRoot;
  const aiNode* rootNode = scene->mRootNode;

  CopyNode(root, rootNode);

  mLoadedModels.insert(std::pair<std::string, ModelTree>(file, modelTree));
  return modelTree;
}

Texture* RenderFrontend::LoadTexture(const std::string &file) {
  auto path = FileLoader::GetRootPath();
  path.append(file);
  //Try to find an existing texture first
  auto it = mLoadedTextures.find(file);
  if (it != mLoadedTextures.end()) {
    return mLoadedTextures.at(file);
  }

  int texwidth, texheight, channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *image = stbi_load(path.generic_string().c_str(), &texwidth, &texheight, &channels, 0);

  if (image == NULL) {
    Log::LogFatal("TEXTURE NOT FOUND: " + file);
    exit(1);
  }
  Texture *t;
  t = m_Backend->LoadTexture(image, texwidth, texheight, channels);

  stbi_image_free(image);

  mLoadedTextures.insert(std::pair<std::string, Texture*>(file, t));

  return t;
}

std::vector<Character> RenderFrontend::LoadFont(const std::string &file) {
  //Try to find cached font first
  auto it = mLoadedFonts.find(file);
  if (it != mLoadedFonts.end()) {
    return mLoadedFonts.at(file);
  }
  std::vector<Character> font;
  FT_Library ft;
  if (FT_Init_FreeType(&ft)) {
    Log::LogFatal("Could not initialize freetype");
    exit(1);
  }
  const std::string absoluteFontPath = FileLoader::GetFilePath(file);
  FT_Face face;
  if (FT_New_Face(ft, absoluteFontPath.c_str(), 0, &face)) {
    Log::LogFatal("COULD NOT LOAD FONT");
    Log::LogFatal(absoluteFontPath);
    exit(1);
  }
  FT_Set_Pixel_Sizes(face, 0, FONT_SIZE);

  for (char c = ' '; c < 127; c++) {
    Character ch;
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
      Log::LogFatal("COULD NOT GET CHARACTER");
    }

    int width = face->glyph->bitmap.width;
    int height = face->glyph->bitmap.rows;
    int bearingY = 2 * face->glyph->bitmap_top - (FONT_SIZE / 2) - height;

    ch.mInternalScale.x = (float)width / FONT_SIZE;
    ch.mInternalScale.y = (float)height / FONT_SIZE;
    ch.mInternalShift.y = (float)bearingY / FONT_SIZE;

    Texture *t = m_Backend->LoadTexture(face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, 1);
    ch.mTexture = t;
    font.push_back(ch);
  }

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  mLoadedFonts.insert(std::pair<std::string, std::vector<Character>>(file, font));
  return font;
}

std::vector<char> RenderFrontend::LoadShaderFile(const std::string &file) {

  //Build the file location based on the backend requirements
  auto path = FileLoader::GetRootPath();
  path.append("shaders");

  path.append(m_Backend->GetShaderFolderName());
  path.append(file);

  /**
  * Read data from shader program file
  */
  std::ifstream shaderFile;
  long shaderFileLength;

  shaderFile.open(path, std::ios::binary);
  if (shaderFile.fail()) {
    Log::LogFatal("ERROR OPENING SHADER FILE");
    Log::LogFatal(path.string());
    exit(1);
  }
  shaderFile.seekg(0, shaderFile.end);
  shaderFileLength = shaderFile.tellg();
  shaderFile.seekg(0, shaderFile.beg);

  std::vector<char> data(shaderFileLength);

  shaderFile.read(data.data(), shaderFileLength);

  shaderFile.close();
  return data;
}

Shader* RenderFrontend::LoadShader(const std::string &vertexFile, const std::string &fragmentFile, const DRAW_STAGE stage) {
  u32 stage_idx = static_cast<u32>(stage);
  auto it = mLoadedShaders.find(vertexFile + fragmentFile + std::to_string(stage_idx));
  if (it != mLoadedShaders.end()) {
    return mLoadedShaders.at(vertexFile + fragmentFile + std::to_string(stage_idx));
  }
  std::vector<char> vertexData = LoadShaderFile(vertexFile);
  std::vector<char> fragmentData = LoadShaderFile(fragmentFile);

  Shader* s = m_Backend->CreateShader(vertexData, fragmentData, stage);
  mLoadedShaders.insert(std::pair<std::string, Shader*>(vertexFile + fragmentFile + std::to_string(stage_idx), s));
  return s;
}

void RenderFrontend::SetShaderUserData(const Mat4 &value) {
  m_ShaderUserData = value;
}

void RenderFrontend::SetCameraShader(const std::string &vertexFile, const std::string &fragmentFile) {
  m_Backend->SetFrameBufferShader(LoadShader(vertexFile, fragmentFile, DRAW_STAGE::UI), DRAW_STAGE::UI);
}

void RenderFrontend::SetCameraUserData(const Mat4 &value) {
  SetShaderUserData(value);
}

void RenderFrontend::DeleteModel(Model &model) {
//  DeleteTexture(model.mTexture);
}

void RenderFrontend::DeleteTexture(Texture* texture) {

}

void RenderFrontend::Draw(const ModelTree &modeltree, const Mat4& rootTransform) {

  std::shared_ptr<Node> node = modeltree.mRoot;
  //Parse tree to get models

  DrawNode(modeltree, node, rootTransform);

}

void RenderFrontend::DrawSprites(const std::vector<Texture *> &sprites, const std::vector<Transform2D> &transforms, const bool isText) {
  if (sprites.size() != transforms.size()) {
    Log::LogFatal("Transform/sprite array size mismatch!");
  }

  for (u32 i = 0; i < sprites.size(); i++) {
    Drawable d;
    d.mShader = isText ? m_TextShader : m_SpriteShader;
    d.mVBuffer = m_UIModel.mVBuffer;
    d.mNumFaces = m_UIModel.mNumFaces;
    d.mTexture = sprites[i];

    float layer = (float)transforms[i].layer / MAX_UI_LAYER;
    float depth = 0.5f;

    if (m_Backend->GetDepthMode() == DEPTH_MODE::ZERO_TO_ONE) {
      const float furthest = 0.7f;
      const float closest = 0.2f;

      depth = layer * (closest - furthest) + furthest;
    }

    Transform t;
    t.position = Vec3(transforms[i].position.x, transforms[i].position.y, depth);
    t.rotation = Vec3(0.0f, 0.0f, transforms[i].rotation);
    t.scale = Vec3(transforms[i].scale.x, -1 * transforms[i].scale.y, 1.0f);

    d.mTransformMatrix = m_AspectMatrix * TransformToMat4(t);

    mUIToDraw.push_back(d);
  }
}

void RenderFrontend::BeginFrame() {
  mWorldToDraw.clear();
  mUIToDraw.clear();

  int mouseX, mouseY;
  u32 mousebutton;
  mousebutton = SDL_GetMouseState(&mouseX, &mouseY);
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos.x = mouseX;
  io.MousePos.y = mouseY;
  io.MouseDown[0] = (mousebutton & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;

  ImGui::NewFrame();
}

void RenderFrontend::EndFrame() {
  Mat4 view = Mat4(1.0f);
  Mat4 proj = Mat4(1.0f);

  if (mainCamera) {
    view = mainCamera->mView;
    proj = mainCamera->mProjection;
  }

  //ImGUI windows

  ImGui::Begin("Render Info");
  if (ImGui::CollapsingHeader("Render Device")) {
    ImGui::Text("%s", m_Backend->GetDeviceName().c_str());
    ImGui::Text("Used VRAM: %u MB", m_Backend->GetUsedVRAM() / 1024 / 1024);
  }

  if (ImGui::CollapsingHeader("Render Statistics:")) {
    ImGui::Text("# 3D Models: %u", mWorldToDraw.size());
  }

  LightData lights;
  lights.mDirectionalLight = m_DirectionalData;


  ImGui::End();

  //Sort UI by depth
  std::map<float, std::vector<Drawable>> sortedUI;
  std::vector<Drawable> ui;
  /*for (const auto& ui : mUIToDraw) {
    float distance = ui.mTransformMatrix[3].z;
    sortedUI[distance].push_back(ui);
  }

  for (auto it = sortedUI.rbegin(); it != sortedUI.rend(); it++) {
    for (auto& d : it->second) {
      ui.push_back(d);
    }
  }*/
  m_Backend->Draw(view, proj, m_ShaderUserData, mWorldToDraw, ui, lights);
}

void RenderFrontend::SetMainCamera(CameraComponent *camera) {
  mainCamera = camera;
}

const IVec2 RenderFrontend::GetScreenResolution() {
  return IVec2(mScreenX, mScreenY);
}

DEPTH_MODE RenderFrontend::GetDepthMode() {
  return m_Backend->GetDepthMode();
}
void RenderFrontend::DrawNode(const ModelTree &modeltree, const std::shared_ptr<Node> &node, const Mat4& parentTransform) {

  for (u32 i = 0; i < node->mMeshIndices.size(); i++) {
    Drawable d;

    d.mShader = modeltree.mShader;
    d.mVBuffer = modeltree.mMeshes[node->mMeshIndices[i]].mVBuffer;
    d.mNumFaces = modeltree.mMeshes[node->mMeshIndices[i]].mNumFaces;
    d.mTexture = modeltree.mMeshes[node->mMeshIndices[i]].mTexture;

    d.mTransformMatrix = parentTransform * node->mTransformMatrix;
    mWorldToDraw.push_back(d);
  }

  for(const auto& child : node->mChildren) {
    DrawNode(modeltree, child, parentTransform * node->mTransformMatrix);
  }
}
void RenderFrontend::SetDirectionalLight(const Vec4 &direction,
                                         const Vec4 &ambientColor,
                                         const Vec4 &diffuseColor,
                                         const Vec4 &specularColor) {
  m_DirectionalData.m_Direction = direction;
  m_DirectionalData.m_AmbientColor = ambientColor;
  m_DirectionalData.m_DiffuseColor = diffuseColor;
  m_DirectionalData.m_SpecularColor = specularColor;

}
