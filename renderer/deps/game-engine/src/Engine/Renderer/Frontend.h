#pragma once

#include "Backend.h"
#include "../CommonTypes.h"
#include "../Components/CameraComponent.h"
#include "Model.h"
#include "FrameBuffer.h"
#include <vector>
#include <map>
#include "Light.h"

//Increasing this will make the font look better, but might take more time to create
const int FONT_SIZE = 256;

/**
* General rendering algorithm for handling world and UI
* 1. Set global view and projection matricies from main camera
* 2. Draw everything marked as a WORLD model type onto offscreen framebuffer
* 3. Draw framebuffer onto quad and overlay models with UI type on top
* 4. Send finished image to screen
*/
class RenderFrontend {
public:

  /*!
  * Sets up renderer window, framebuffer, and other global state
  */
  static void Init();

  /*!
  * Shuts down window, framebuffer, and global state
  */
  static void Shutdown();

  /*!
  * Stores the model located at the given file location and returns a handle to the model
  * @param[in] file The file name to load, relative to the data folder
  * @return A vector prefilled with Model handles
  */
  static ModelTree LoadModel(const std::string &file);

  /*!
  * Stores the texture located at the given file location and returns a handle to the texture
  * @param[in] file The file name to load, relative to the data folder
  * @return The handle to the texture
  */
  static Texture* LoadTexture(const std::string &file);

  /*!
  * Stores the font data located at the given file location and returns a handle to the font
  * @param[in] file The file name to load, relative to the data folder
  * @return A vector prefilled with the font character handles
  */
  static std::vector<Character> LoadFont(const std::string &file);

  /*!
  * Stores the shader located at the given file location and returns a handle to the shader
  * @param[in] vertexFile The vertex shader file name to load, relative to the data folder
  * @param[in] fragmentFile The frament shader file name to load, relative to the data folder
  * @return The handle to the shader
  */
  static Shader* LoadShader(const std::string &vertexFile, const std::string &fragmentFile, const DRAW_STAGE stage);

  /*!
  * Sets the user data uniform in the given shader object
  * @param[in] value The Mat4 value to set
  */
  static void SetShaderUserData(const Mat4 &value);

  /*!
  * Sets the post process shader for the 3D geometry pass
  * @param[in] vertexFile The vertex shader file path to use, relative to the data folder
  * @param[in] fragmentFile The fragment shader file path to use, relative to the data folder
  */
  static void SetCameraShader(const std::string &vertexFile, const std::string &fragmentFile);

  /*!
  * Sets a uniform property for the 3D post process shader
  * @param[in] value The Mat4 value to set
  */
  static void SetCameraUserData(const Mat4 &value);

  /*!
  * Frees the model from GPU memory
  * @param[in] model The model handle to free
  */
  static void DeleteModel(Model &model);

  /*!
  * Frees the texture from GPU memory
  * @param[in] model The texture handle to free
  */
  static void DeleteTexture(Texture* texture);

  /*!
  * Queues the provided model for drawing, but may actually draw until EndFrame is called
  * @param[in] model The model to queue for drawing
  */
  static void Draw(const ModelTree &modeltree, const Mat4& rootTransform);

  static void DrawSprites(const std::vector<Texture*> &sprites, const std::vector<Transform2D> &transforms, const bool isText);

  /*!
   * Sets the directional light for the scene
   * @param direction Direction of the light
   * @param ambientColor Ambient color of the light
   * @param diffuseColor Diffuse color of the light
   * @param specularColor Specular color of the light
   */
  static void SetDirectionalLight(const Vec4& direction, const Vec4& ambientColor, const Vec4& diffuseColor, const Vec4& specularColor);

  /*!
  * Binds the necessary framebuffer and other resources to begin drawing
  */
  static void BeginFrame();

  /*!
  * Empties any drawing queues and presents the final image to the screen
  */
  static void EndFrame();

  /*!
  * Sets the current main camera, for obtaining view and projection matricies
  * @param[in] camera Pointer to the camera component to use
  */
  static void SetMainCamera(CameraComponent *camera);

  /*!
  * Gets the current output resolution, output as a IVec2
  * @return A 2D vector where X=width, Y=height
  */
  static const IVec2 GetScreenResolution();

  /*!
  * Gets the current depth mode of the backend
  * @return The backen's depth mode
  */
  static DEPTH_MODE GetDepthMode();
private:
  static RenderBackend* m_Backend;

  /**
  * Cache for already loaded assets, mapping the file name to the loaded data
  */
  static std::map<std::string, ModelTree> mLoadedModels;
  static std::map<std::string, Texture*> mLoadedTextures;
  static std::map<std::string, std::vector<Character>> mLoadedFonts;
  static std::map<std::string, Shader*> mLoadedShaders;

  static int mScreenX, mScreenY;

  static CameraComponent* mainCamera;

  /**
  * Scene we are drawing this frame
  */
  static std::vector<Drawable> mWorldToDraw;
  static std::vector<Drawable> mUIToDraw;

  static std::vector<char> LoadShaderFile(const std::string &file);

  static void DrawNode(const ModelTree &modeltree, const std::shared_ptr<Node>& node, const Mat4& parentTransform);

  static bool m_DrawUI;
  static Mat4 m_ShaderUserData;

  static Shader* m_TextShader;
  static Shader* m_SpriteShader;
  static Model m_UIModel;

  static Mat4 m_AspectMatrix;
  
  static DirectionalLightData m_DirectionalData;
};