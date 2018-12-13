#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include <memory>

class Component {
public:
  virtual ~Component();
  virtual void Update(const float deltaTime);

  template<typename T>
  std::shared_ptr<T> AddChild() {
    std::shared_ptr<T> child = std::make_shared<T>();
    mChildren.push_back(child);
    return child;
  }

private:
  std::vector<std::shared_ptr<Component>> mChildren;
};


#endif
