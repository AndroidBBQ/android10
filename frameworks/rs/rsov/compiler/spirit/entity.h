/*
 * Copyright 2017, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <vector>

#include "word_stream.h"

namespace android {
namespace spirit {

class Builder;
class IVisitor;

class Entity {
public:
  Entity() {}
  Entity(Builder *b) : mBuilder(b) {}

  virtual ~Entity() {}

  virtual void accept(IVisitor *visitor) = 0;
  virtual bool DeserializeInternal(InputWordStream &IS) = 0;
  virtual void Serialize(OutputWordStream &OS) const;
  virtual void dump() const {}

  void setBuilder(Builder *builder) { mBuilder = builder; }

protected:
  Builder *mBuilder;
};

template <typename T> T *Deserialize(InputWordStream &IS) {
  std::unique_ptr<T> entity(new T());
  if (!entity->DeserializeInternal(IS)) {
    return nullptr;
  }
  return entity.release();
}

template <typename T> T *Deserialize(const std::vector<uint32_t> &words) {
  std::unique_ptr<InputWordStream> IS(InputWordStream::Create(words));
  return Deserialize<T>(*IS);
}

template <class T>
void DeserializeZeroOrMore(InputWordStream &IS, std::vector<T *> &all) {
  while (auto entity = Deserialize<T>(IS)) {
    all.push_back(entity);
  }
}

template <class T>
std::vector<uint32_t> Serialize(T* e) {
  std::unique_ptr<OutputWordStream> OS(OutputWordStream::Create());
  e->Serialize(*OS);
  return OS->getWords();
}

} // namespace spirit
} // namespace android

#endif // ENTITY_H
