/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_info/device_info.h"

#include <sys/system_properties.h>
#include <EGL/egl.h>
#include <GLES3/gl32.h>

#include <cstdint>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <set>

namespace {
using ProtoInfoWithErrors = androidgamesdk_deviceinfo::InfoWithErrors;
using ProtoErrors         = androidgamesdk_deviceinfo::Errors;
using ProtoInfo           = androidgamesdk_deviceinfo::Info;
using ProtoCpuCore        = androidgamesdk_deviceinfo::Info::CpuCore;
using ProtoGl             = androidgamesdk_deviceinfo::Info::Gl;

// size of GL view and texture in future
constexpr int VIEW_WIDTH = 8;
constexpr int VIEW_HEIGHT = VIEW_WIDTH;

namespace string_util {
bool startsWith(const std::string& text, const std::string& start) {
  return text.compare(0, start.length(), start) == 0;
}

void splitAdd(const std::string& toSplit, char delimeter,
              std::set<std::string>* result) {
  std::istringstream istr(toSplit);
  std::string piece;
  while (std::getline(istr, piece, delimeter)) {
    if (piece.length() > 0) {
      result->insert(piece);
    }
  }
}
}  // namespace string_util

template <typename T>
T readFile(const std::string& fileName, const T& error) {
  std::ifstream reader(fileName);
  if (reader.fail()) return error;
  T result;
  reader >> result;
  if (reader.fail()) return error;
  return result;
}

std::string readCpuPresent() {
  const std::string ERROR = "ERROR";
  return readFile("/sys/devices/system/cpu/present", ERROR);
}
std::string readCpuPossible() {
  const std::string ERROR = "ERROR";
  return readFile("/sys/devices/system/cpu/possible", ERROR);
}
int readCpuIndexMax() {
  constexpr int ERROR = -1;
  return readFile("/sys/devices/system/cpu/kernel_max", ERROR);
}
int64_t readCpuFreqMax(int cpuIndex) {
  const std::string fileName =
    "/sys/devices/system/cpu/cpu" +
    std::to_string(cpuIndex) +
    "/cpufreq/cpuinfo_max_freq";
  constexpr int64_t ERROR = -1;
  return readFile(fileName, ERROR);
}

// returns number of errors
int readHardware(std::vector<std::string>& result, ProtoErrors& errors) {
  std::ifstream f("/proc/cpuinfo");
  if (f.fail()){
    errors.set_hardware("Could not read.");
    return 1;
  }
  const std::string FIELD_KEY = "Hardware\t: ";
  std::string line;
  while (std::getline(f, line)) {
    if (::string_util::startsWith(line, FIELD_KEY)) {
      std::string val = line.substr(FIELD_KEY.length(), std::string::npos);
      result.push_back(val);
    }
  }
  return 0;
}

// returns number of errors
int readFeatures(std::set<std::string>& result, ProtoErrors& errors) {
  std::ifstream f("/proc/cpuinfo");
  if (f.fail()){
    errors.set_features("Could not read.");
    return 1;
  }
  const std::string FIELD_KEY = "Features\t: ";
  std::string line;
  while (std::getline(f, line)) {
    if (::string_util::startsWith(line, FIELD_KEY)) {
      std::string features = line.substr(FIELD_KEY.length(), std::string::npos);
      ::string_util::splitAdd(features, ' ', &result);
    }
  }
  return 0;
}

std::string getSystemPropViaGet(const char* key, ::ProtoErrors& errors) {
  char buffer[PROP_VALUE_MAX + 1];  // +1 for terminator
  int bufferLen = __system_property_get(key, buffer);
  if (bufferLen > PROP_VALUE_MAX){
    const std::string HEADER = "Overflow: ";
    errors.add_system_props(HEADER + key);
    return "";
  }
  return std::string(buffer, bufferLen);
}

std::string getSystemPropViaReadCallback(const char* key) {
  const prop_info* pi = __system_property_find(key);
  if (pi == nullptr) {
    return "";
  }
  std::string result;
  __system_property_read_callback(pi,
    [](void* cookie, const char*, const char* value, unsigned) {
      auto sysOut = reinterpret_cast<std::string*>(cookie);
      *sysOut = value;
    },
    &result
  );
  return result;
}

std::string getSystemProp(const char* key,
  ::ProtoErrors& errors, bool useCallbackApi) {
  if (useCallbackApi) {
    return getSystemPropViaReadCallback(key);
  } else {
    return getSystemPropViaGet(key, errors);
  }
}

// returns number of errors
int addSystemProperties(::ProtoInfo& info, ::ProtoErrors& errors) {
  std::string sdkVersionString =
    getSystemPropViaGet("ro.build.version.sdk", errors);
  info.set_ro_build_version_sdk(sdkVersionString);

  int sdkVersion = atoi(sdkVersionString.c_str());
  bool useCallbackApi = (26 <= sdkVersion);
  info.set_ro_chipname(
    getSystemProp("ro.chipname", errors, useCallbackApi));
  info.set_ro_board_platform(
    getSystemProp("ro.board.platform", errors, useCallbackApi));
  info.set_ro_product_board(
    getSystemProp("ro.product.board", errors, useCallbackApi));
  info.set_ro_mediatek_platform(
    getSystemProp("ro.mediatek.platform", errors, useCallbackApi));
  info.set_ro_arch(
    getSystemProp("ro.arch", errors, useCallbackApi));
  info.set_ro_build_fingerprint(
    getSystemProp("ro.build.fingerprint", errors, useCallbackApi));

  return errors.system_props_size();
}

// returns number of errors
int checkEglError(const char* title, ::ProtoErrors& errors) {
  EGLint error = eglGetError();
  if (error == EGL_SUCCESS) return 0;
  std::stringstream ss;
  ss << title << ": 0x" << std::hex << (int)error << std::dec;
  errors.set_egl(ss.str());
  return 1;
}

int flushGlErrors(::ProtoErrors& errors) {
  int numErrors = 0;
  while (GLenum e = glGetError() != GL_NO_ERROR) {
    std::stringstream ss;
    ss << "0x" << std::hex << (int)e << std::dec;
    errors.add_gl(ss.str());
    numErrors++;
  }
  return numErrors;
}

// returns number of errors
int setupEGl(::ProtoInfoWithErrors& proto) {
  ProtoErrors& errors = *proto.mutable_errors();

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (int numErrors = checkEglError("eglGetDisplay", errors)){
    return numErrors;
  }

  eglInitialize(display, nullptr, nullptr);  // do not care about egl version
  if (int numErrors = checkEglError("eglInitialize", errors)){
    return numErrors;
  }

  EGLint configAttribs[] = {
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_NONE
  };
  EGLConfig config;
  EGLint numConfigs = -1;
  eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
  if (int numErrors = checkEglError("eglChooseConfig", errors)){
    return numErrors;
  }

  EGLint contextAttribs[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  EGLContext context =
    eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (int numErrors = checkEglError("eglCreateContext", errors)){
    return numErrors;
  }

  EGLint pbufferAttribs[] = {
    EGL_WIDTH,  VIEW_WIDTH,
    EGL_HEIGHT, VIEW_HEIGHT,
    EGL_NONE
  };
  EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
  if (int numErrors = checkEglError("eglCreatePbufferSurface", errors)){
    return numErrors;
  }

  eglMakeCurrent(display, surface, surface, context);
  if (int numErrors = checkEglError("eglMakeCurrent", errors)){
    return numErrors;
  }

  return 0;
}

namespace gl_util {
typedef const GLubyte* GlStr;
typedef GlStr(*FuncTypeGlGetstringi)(GLenum, GLint);
FuncTypeGlGetstringi glGetStringi = 0;
const char* getStringIndexed(GLenum e, GLuint index){
  return reinterpret_cast<const char*>(::gl_util::glGetStringi(e, index));
}

typedef void(*FuncTypeGlGetInteger64v)(GLenum, GLint64*);
FuncTypeGlGetInteger64v glGetInteger64v = 0;
GLint64 getInt64(GLenum e) {
  GLint64 result = -1;
  glGetInteger64v(e, &result);
  return result;
}

typedef void(*FuncTypeGlGetIntegeri_v)(GLenum, GLuint, GLint*);
FuncTypeGlGetIntegeri_v glGetIntegeri_v = 0;
GLint getIntIndexed(GLenum e, GLuint index) {
  GLint result = -1;
  glGetIntegeri_v(e, index, &result);
  return result;
}

const char* getString(GLenum e){
  return reinterpret_cast<const char*>(glGetString(e));
}

GLfloat getFloat(GLenum e) {
  GLfloat result = -1;
  glGetFloatv(e, &result);
  return result;
}
GLint getInt(GLenum e) {
  GLint result = -1;
  glGetIntegerv(e, &result);
  return result;
}
GLboolean getBool(GLenum e) {
  GLboolean result = false;
  glGetBooleanv(e, &result);
  return result;
}
}  // namespace gl_util

void addGlConstsV2_0(::ProtoGl& gl) {
  gl.set_gl_aliased_line_width_range(
    ::gl_util::getFloat(GL_ALIASED_LINE_WIDTH_RANGE));
  gl.set_gl_aliased_point_size_range(
    ::gl_util::getFloat(GL_ALIASED_POINT_SIZE_RANGE));
  gl.set_gl_max_combined_texture_image_units(
    ::gl_util::getInt(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_cube_map_texture_size(
    ::gl_util::getInt(GL_MAX_CUBE_MAP_TEXTURE_SIZE));
  gl.set_gl_max_fragment_uniform_vectors(
    ::gl_util::getInt(GL_MAX_FRAGMENT_UNIFORM_VECTORS));
  gl.set_gl_max_renderbuffer_size(
    ::gl_util::getInt(GL_MAX_RENDERBUFFER_SIZE));
  gl.set_gl_max_texture_image_units(
    ::gl_util::getInt(GL_MAX_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_texture_size(
    ::gl_util::getInt(GL_MAX_TEXTURE_SIZE));
  gl.set_gl_max_varying_vectors(
    ::gl_util::getInt(GL_MAX_VARYING_VECTORS));
  gl.set_gl_max_vertex_attribs(
    ::gl_util::getInt(GL_MAX_VERTEX_ATTRIBS));
  gl.set_gl_max_vertex_texture_image_units(
    ::gl_util::getInt(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_vertex_uniform_vectors(
    ::gl_util::getInt(GL_MAX_VERTEX_UNIFORM_VECTORS));
  gl.set_gl_max_viewport_dims(
    ::gl_util::getInt(GL_MAX_VIEWPORT_DIMS));
  gl.set_gl_shader_compiler(
    ::gl_util::getBool(GL_SHADER_COMPILER));
  gl.set_gl_subpixel_bits(
    ::gl_util::getInt(GL_SUBPIXEL_BITS));

  GLint numCompressedFormats =
    ::gl_util::getInt(GL_NUM_COMPRESSED_TEXTURE_FORMATS);
  gl.set_gl_num_compressed_texture_formats(numCompressedFormats);
  std::vector<GLint> compressedFormats(numCompressedFormats);
  glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, &compressedFormats[0]);
  for (auto x : compressedFormats) {
    gl.add_gl_compressed_texture_formats(x);
  }

  GLint numShaderBinFormats = ::gl_util::getInt(GL_NUM_SHADER_BINARY_FORMATS);
  gl.set_gl_num_shader_binary_formats(numShaderBinFormats);
  std::vector<GLint> shaderBinFormats(numShaderBinFormats);
  glGetIntegerv(GL_SHADER_BINARY_FORMATS, &shaderBinFormats[0]);
  for (auto x : shaderBinFormats) {
    gl.add_gl_shader_binary_formats(x);
  }

  // shader precision formats
  GLint spfr = -1;  // range
  GLint spfp = -1;  // precision
  glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_LOW_FLOAT, &spfr, &spfp);
  gl.set_spf_vertex_float_low_range(spfr);
  gl.set_spf_vertex_float_low_prec(spfp);
  glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_MEDIUM_FLOAT, &spfr, &spfp);
  gl.set_spf_vertex_float_med_range(spfr);
  gl.set_spf_vertex_float_med_prec(spfp);
  glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_HIGH_FLOAT, &spfr, &spfp);
  gl.set_spf_vertex_float_hig_range(spfr);
  gl.set_spf_vertex_float_hig_prec(spfp);
  glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_LOW_INT, &spfr, &spfp);
  gl.set_spf_vertex_int_low_range(spfr);
  gl.set_spf_vertex_int_low_prec(spfp);
  glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_MEDIUM_INT, &spfr, &spfp);
  gl.set_spf_vertex_int_med_range(spfr);
  gl.set_spf_vertex_int_med_prec(spfp);
  glGetShaderPrecisionFormat(GL_VERTEX_SHADER, GL_HIGH_INT, &spfr, &spfp);
  gl.set_spf_vertex_int_hig_range(spfr);
  gl.set_spf_vertex_int_hig_prec(spfp);
  glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_LOW_FLOAT, &spfr, &spfp);
  gl.set_spf_fragment_float_low_range(spfr);
  gl.set_spf_fragment_float_low_prec(spfp);
  glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT, &spfr, &spfp);
  gl.set_spf_fragment_float_med_range(spfr);
  gl.set_spf_fragment_float_med_prec(spfp);
  glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_FLOAT, &spfr, &spfp);
  gl.set_spf_fragment_float_hig_range(spfr);
  gl.set_spf_fragment_float_hig_prec(spfp);
  glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_LOW_INT, &spfr, &spfp);
  gl.set_spf_fragment_int_low_range(spfr);
  gl.set_spf_fragment_int_low_prec(spfp);
  glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_MEDIUM_INT, &spfr, &spfp);
  gl.set_spf_fragment_int_med_range(spfr);
  gl.set_spf_fragment_int_med_prec(spfp);
  glGetShaderPrecisionFormat(GL_FRAGMENT_SHADER, GL_HIGH_INT, &spfr, &spfp);
  gl.set_spf_fragment_int_hig_range(spfr);
  gl.set_spf_fragment_int_hig_prec(spfp);
}
void addGlConstsV3_0(::ProtoGl& gl) {
  gl.set_gl_max_3d_texture_size(
    ::gl_util::getInt(GL_MAX_3D_TEXTURE_SIZE));
  gl.set_gl_max_array_texture_layers(
    ::gl_util::getInt(GL_MAX_ARRAY_TEXTURE_LAYERS));
  gl.set_gl_max_color_attachments(
    ::gl_util::getInt(GL_MAX_COLOR_ATTACHMENTS));
  gl.set_gl_max_combined_uniform_blocks(
    ::gl_util::getInt(GL_MAX_COMBINED_UNIFORM_BLOCKS));
  gl.set_gl_max_draw_buffers(
    ::gl_util::getInt(GL_MAX_DRAW_BUFFERS));
  gl.set_gl_max_elements_indices(
    ::gl_util::getInt(GL_MAX_ELEMENTS_INDICES));
  gl.set_gl_max_elements_vertices(
    ::gl_util::getInt(GL_MAX_ELEMENTS_VERTICES));
  gl.set_gl_max_fragment_input_components(
    ::gl_util::getInt(GL_MAX_FRAGMENT_INPUT_COMPONENTS));
  gl.set_gl_max_fragment_uniform_blocks(
    ::gl_util::getInt(GL_MAX_FRAGMENT_UNIFORM_BLOCKS));
  gl.set_gl_max_fragment_uniform_components(
    ::gl_util::getInt(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS));
  gl.set_gl_max_program_texel_offset(
    ::gl_util::getInt(GL_MAX_PROGRAM_TEXEL_OFFSET));
  gl.set_gl_max_transform_feedback_interleaved_components(
    ::gl_util::getInt(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS));
  gl.set_gl_max_transform_feedback_separate_attribs(
    ::gl_util::getInt(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS));
  gl.set_gl_max_transform_feedback_separate_components(
    ::gl_util::getInt(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS));
  gl.set_gl_max_uniform_buffer_bindings(
    ::gl_util::getInt(GL_MAX_UNIFORM_BUFFER_BINDINGS));
  gl.set_gl_max_varying_components(
    ::gl_util::getInt(GL_MAX_VARYING_COMPONENTS));
  gl.set_gl_max_vertex_output_components(
    ::gl_util::getInt(GL_MAX_VERTEX_OUTPUT_COMPONENTS));
  gl.set_gl_max_vertex_uniform_blocks(
    ::gl_util::getInt(GL_MAX_VERTEX_UNIFORM_BLOCKS));
  gl.set_gl_max_vertex_uniform_components(
    ::gl_util::getInt(GL_MAX_VERTEX_UNIFORM_COMPONENTS));
  gl.set_gl_min_program_texel_offset(
    ::gl_util::getInt(GL_MIN_PROGRAM_TEXEL_OFFSET));
  gl.set_gl_uniform_buffer_offset_alignment(
    ::gl_util::getInt(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT));
  gl.set_gl_max_samples(
    ::gl_util::getInt(GL_MAX_SAMPLES));

  gl.set_gl_max_texture_lod_bias(::gl_util::getFloat(GL_MAX_TEXTURE_LOD_BIAS));

  ::gl_util::glGetInteger64v =
    reinterpret_cast<::gl_util::FuncTypeGlGetInteger64v>(
      eglGetProcAddress("glGetInteger64v"));
  gl.set_gl_max_combined_fragment_uniform_components(
    ::gl_util::getInt64(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS));
  gl.set_gl_max_element_index(
    ::gl_util::getInt64(GL_MAX_ELEMENT_INDEX));
  gl.set_gl_max_server_wait_timeout(
    ::gl_util::getInt64(GL_MAX_SERVER_WAIT_TIMEOUT));
  gl.set_gl_max_uniform_block_size(
    ::gl_util::getInt64(GL_MAX_UNIFORM_BLOCK_SIZE));
}
void addGlConstsV3_1(::ProtoGl& gl) {
  gl.set_gl_max_atomic_counter_buffer_bindings(
    ::gl_util::getInt(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS));
  gl.set_gl_max_atomic_counter_buffer_size(
    ::gl_util::getInt(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE));
  gl.set_gl_max_color_texture_samples(
    ::gl_util::getInt(GL_MAX_COLOR_TEXTURE_SAMPLES));
  gl.set_gl_max_combined_atomic_counters(
    ::gl_util::getInt(GL_MAX_COMBINED_ATOMIC_COUNTERS));
  gl.set_gl_max_combined_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_combined_compute_uniform_components(
    ::gl_util::getInt(GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS));
  gl.set_gl_max_combined_image_uniforms(
    ::gl_util::getInt(GL_MAX_COMBINED_IMAGE_UNIFORMS));
  gl.set_gl_max_combined_shader_output_resources(
    ::gl_util::getInt(GL_MAX_COMBINED_SHADER_OUTPUT_RESOURCES));
  gl.set_gl_max_combined_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_COMBINED_SHADER_STORAGE_BLOCKS));
  gl.set_gl_max_compute_atomic_counters(
    ::gl_util::getInt(GL_MAX_COMPUTE_ATOMIC_COUNTERS));
  gl.set_gl_max_compute_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_compute_image_uniforms(
    ::gl_util::getInt(GL_MAX_COMPUTE_IMAGE_UNIFORMS));
  gl.set_gl_max_compute_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS));
  gl.set_gl_max_compute_shared_memory_size(
    ::gl_util::getInt(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE));
  gl.set_gl_max_compute_texture_image_units(
    ::gl_util::getInt(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_compute_uniform_blocks(
    ::gl_util::getInt(GL_MAX_COMPUTE_UNIFORM_BLOCKS));
  gl.set_gl_max_compute_uniform_components(
    ::gl_util::getInt(GL_MAX_COMPUTE_UNIFORM_COMPONENTS));
  gl.set_gl_max_compute_work_group_invocations(
    ::gl_util::getInt(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS));
  gl.set_gl_max_depth_texture_samples(
    ::gl_util::getInt(GL_MAX_DEPTH_TEXTURE_SAMPLES));
  gl.set_gl_max_fragment_atomic_counters(
    ::gl_util::getInt(GL_MAX_FRAGMENT_ATOMIC_COUNTERS));
  gl.set_gl_max_fragment_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_fragment_image_uniforms(
    ::gl_util::getInt(GL_MAX_FRAGMENT_IMAGE_UNIFORMS));
  gl.set_gl_max_fragment_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS));
  gl.set_gl_max_framebuffer_height(
    ::gl_util::getInt(GL_MAX_FRAMEBUFFER_HEIGHT));
  gl.set_gl_max_framebuffer_samples(
    ::gl_util::getInt(GL_MAX_FRAMEBUFFER_SAMPLES));
  gl.set_gl_max_framebuffer_width(
    ::gl_util::getInt(GL_MAX_FRAMEBUFFER_WIDTH));
  gl.set_gl_max_image_units(
    ::gl_util::getInt(GL_MAX_IMAGE_UNITS));
  gl.set_gl_max_integer_samples(
    ::gl_util::getInt(GL_MAX_INTEGER_SAMPLES));
  gl.set_gl_max_program_texture_gather_offset(
    ::gl_util::getInt(GL_MAX_PROGRAM_TEXTURE_GATHER_OFFSET));
  gl.set_gl_max_sample_mask_words(
    ::gl_util::getInt(GL_MAX_SAMPLE_MASK_WORDS));
  gl.set_gl_max_shader_storage_buffer_bindings(
    ::gl_util::getInt(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS));
  gl.set_gl_max_uniform_locations(
    ::gl_util::getInt(GL_MAX_UNIFORM_LOCATIONS));
  gl.set_gl_max_vertex_atomic_counters(
    ::gl_util::getInt(GL_MAX_VERTEX_ATOMIC_COUNTERS));
  gl.set_gl_max_vertex_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_vertex_attrib_bindings(
    ::gl_util::getInt(GL_MAX_VERTEX_ATTRIB_BINDINGS));
  gl.set_gl_max_vertex_attrib_relative_offset(
    ::gl_util::getInt(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET));
  gl.set_gl_max_vertex_attrib_stride(
    ::gl_util::getInt(GL_MAX_VERTEX_ATTRIB_STRIDE));
  gl.set_gl_max_vertex_image_uniforms(
    ::gl_util::getInt(GL_MAX_VERTEX_IMAGE_UNIFORMS));
  gl.set_gl_max_vertex_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS));
  gl.set_gl_min_program_texture_gather_offset(
    ::gl_util::getInt(GL_MIN_PROGRAM_TEXTURE_GATHER_OFFSET));
  gl.set_gl_shader_storage_buffer_offset_alignment(
    ::gl_util::getInt(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT));

  gl.set_gl_max_shader_storage_block_size(
    ::gl_util::getInt64(GL_MAX_SHADER_STORAGE_BLOCK_SIZE));

  ::gl_util::glGetIntegeri_v =
    reinterpret_cast<::gl_util::FuncTypeGlGetIntegeri_v>(
      eglGetProcAddress("glGetIntegeri_v"));
  gl.set_gl_max_compute_work_group_count_0(
    ::gl_util::getIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0));
  gl.set_gl_max_compute_work_group_count_1(
    ::gl_util::getIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1));
  gl.set_gl_max_compute_work_group_count_2(
    ::gl_util::getIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2));
  gl.set_gl_max_compute_work_group_size_0(
    ::gl_util::getIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0));
  gl.set_gl_max_compute_work_group_size_1(
    ::gl_util::getIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1));
  gl.set_gl_max_compute_work_group_size_2(
    ::gl_util::getIntIndexed(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2));
}
void addGlConstsV3_2(::ProtoGl& gl) {
  gl.set_gl_context_flags(
    ::gl_util::getInt(GL_CONTEXT_FLAGS));
  gl.set_gl_fragment_interpolation_offset_bits(
    ::gl_util::getInt(GL_FRAGMENT_INTERPOLATION_OFFSET_BITS));
  gl.set_gl_layer_provoking_vertex(
    ::gl_util::getInt(GL_LAYER_PROVOKING_VERTEX));
  gl.set_gl_max_combined_geometry_uniform_components(
    ::gl_util::getInt(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS));
  gl.set_gl_max_combined_tess_control_uniform_components(
    ::gl_util::getInt(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS));
  gl.set_gl_max_combined_tess_evaluation_uniform_components(
    ::gl_util::getInt(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS));
  gl.set_gl_max_debug_group_stack_depth(
    ::gl_util::getInt(GL_MAX_DEBUG_GROUP_STACK_DEPTH));
  gl.set_gl_max_debug_logged_messages(
    ::gl_util::getInt(GL_MAX_DEBUG_LOGGED_MESSAGES));
  gl.set_gl_max_debug_message_length(
    ::gl_util::getInt(GL_MAX_DEBUG_MESSAGE_LENGTH));
  gl.set_gl_max_framebuffer_layers(
    ::gl_util::getInt(GL_MAX_FRAMEBUFFER_LAYERS));
  gl.set_gl_max_geometry_atomic_counters(
    ::gl_util::getInt(GL_MAX_GEOMETRY_ATOMIC_COUNTERS));
  gl.set_gl_max_geometry_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_geometry_image_uniforms(
    ::gl_util::getInt(GL_MAX_GEOMETRY_IMAGE_UNIFORMS));
  gl.set_gl_max_geometry_input_components(
    ::gl_util::getInt(GL_MAX_GEOMETRY_INPUT_COMPONENTS));
  gl.set_gl_max_geometry_output_components(
    ::gl_util::getInt(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS));
  gl.set_gl_max_geometry_output_vertices(
    ::gl_util::getInt(GL_MAX_GEOMETRY_OUTPUT_VERTICES));
  gl.set_gl_max_geometry_shader_invocations(
    ::gl_util::getInt(GL_MAX_GEOMETRY_SHADER_INVOCATIONS));
  gl.set_gl_max_geometry_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS));
  gl.set_gl_max_geometry_texture_image_units(
    ::gl_util::getInt(GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_geometry_total_output_components(
    ::gl_util::getInt(GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS));
  gl.set_gl_max_geometry_uniform_blocks(
    ::gl_util::getInt(GL_MAX_GEOMETRY_UNIFORM_BLOCKS));
  gl.set_gl_max_geometry_uniform_components(
    ::gl_util::getInt(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS));
  gl.set_gl_max_label_length(
    ::gl_util::getInt(GL_MAX_LABEL_LENGTH));
  gl.set_gl_max_patch_vertices(
    ::gl_util::getInt(GL_MAX_PATCH_VERTICES));
  gl.set_gl_max_tess_control_atomic_counters(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS));
  gl.set_gl_max_tess_control_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_tess_control_image_uniforms(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_IMAGE_UNIFORMS));
  gl.set_gl_max_tess_control_input_components(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_INPUT_COMPONENTS));
  gl.set_gl_max_tess_control_output_components(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS));
  gl.set_gl_max_tess_control_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_SHADER_STORAGE_BLOCKS));
  gl.set_gl_max_tess_control_texture_image_units(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_tess_control_total_output_components(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS));
  gl.set_gl_max_tess_control_uniform_blocks(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS));
  gl.set_gl_max_tess_control_uniform_components(
    ::gl_util::getInt(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS));
  gl.set_gl_max_tess_evaluation_atomic_counters(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS));
  gl.set_gl_max_tess_evaluation_atomic_counter_buffers(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS));
  gl.set_gl_max_tess_evaluation_image_uniforms(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_IMAGE_UNIFORMS));
  gl.set_gl_max_tess_evaluation_input_components(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS));
  gl.set_gl_max_tess_evaluation_output_components(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS));
  gl.set_gl_max_tess_evaluation_shader_storage_blocks(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_SHADER_STORAGE_BLOCKS));
  gl.set_gl_max_tess_evaluation_texture_image_units(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS));
  gl.set_gl_max_tess_evaluation_uniform_blocks(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS));
  gl.set_gl_max_tess_evaluation_uniform_components(
    ::gl_util::getInt(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS));
  gl.set_gl_max_tess_gen_level(
    ::gl_util::getInt(GL_MAX_TESS_GEN_LEVEL));
  gl.set_gl_max_tess_patch_components(
    ::gl_util::getInt(GL_MAX_TESS_PATCH_COMPONENTS));
  gl.set_gl_max_texture_buffer_size(
    ::gl_util::getInt(GL_MAX_TEXTURE_BUFFER_SIZE));
  gl.set_gl_texture_buffer_offset_alignment(
    ::gl_util::getInt(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT));
  gl.set_gl_reset_notification_strategy(
    ::gl_util::getInt(GL_RESET_NOTIFICATION_STRATEGY));
  gl.set_gl_max_fragment_interpolation_offset(
    ::gl_util::getFloat(GL_MAX_FRAGMENT_INTERPOLATION_OFFSET));
  gl.set_gl_min_fragment_interpolation_offset(
    ::gl_util::getFloat(GL_MIN_FRAGMENT_INTERPOLATION_OFFSET));
  gl.set_gl_multisample_line_width_granularity(
    ::gl_util::getFloat(GL_MULTISAMPLE_LINE_WIDTH_GRANULARITY));
  gl.set_gl_multisample_line_width_range(
    ::gl_util::getFloat(GL_MULTISAMPLE_LINE_WIDTH_RANGE));

  gl.set_gl_primitive_restart_for_patches_supported(
    ::gl_util::getBool(GL_PRIMITIVE_RESTART_FOR_PATCHES_SUPPORTED));
}

// returns number of errors
int addGl(::ProtoInfoWithErrors& proto) {
  int numErrors = 0;

  ::ProtoInfo& info = *proto.mutable_info();
  ::ProtoGl& gl = *info.mutable_gl();
  ::ProtoErrors& errors = *proto.mutable_errors();

  gl.set_renderer(::gl_util::getString(GL_RENDERER));
  gl.set_vendor(::gl_util::getString(GL_VENDOR));
  gl.set_version(::gl_util::getString(GL_VERSION));
  gl.set_shading_language_version(
    ::gl_util::getString(GL_SHADING_LANGUAGE_VERSION));

  numErrors += flushGlErrors(errors);

  GLint glVerMajor = -1;
  GLint glVerMinor = -1;
  glGetIntegerv(GL_MAJOR_VERSION, &glVerMajor);
  // if GL_MAJOR_VERSION is not recognized, assume version 2.0
  if (glGetError() != GL_NO_ERROR) {
    glVerMajor = 2;
    glVerMinor = 0;
  } else {
    glGetIntegerv(GL_MINOR_VERSION, &glVerMinor);
  }
  gl.set_version_major(glVerMajor);
  gl.set_version_minor(glVerMinor);

  // gl extensions
  if (glVerMajor >= 3) {
    int numExts = -1;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
    ::gl_util::glGetStringi = reinterpret_cast<::gl_util::FuncTypeGlGetstringi>(
                                eglGetProcAddress("glGetStringi"));
    for (int i = 0; i < numExts; i++) {
      std::string s = ::gl_util::getStringIndexed(GL_EXTENSIONS, i);
      gl.add_extension(s);
    }
  } else {
    std::string exts = ::gl_util::getString(GL_EXTENSIONS);
    std::set<std::string> split;
    ::string_util::splitAdd(exts, ' ', &split);
    for (const std::string& s : split) {
      gl.add_extension(s);
    }
  }

  if (glVerMajor > 2 || (glVerMajor == 2 && glVerMinor >= 0)) {  // >= gles 2.0
    addGlConstsV2_0(gl);
  }
  if (glVerMajor > 3 || (glVerMajor == 3 && glVerMinor >= 0)) {  // >= gles 3.0
    addGlConstsV3_0(gl);
  }
  if (glVerMajor > 3 || (glVerMajor == 3 && glVerMinor >= 1)) {  // >= gles 3.1
    addGlConstsV3_1(gl);
  }
  if (glVerMajor > 3 || (glVerMajor == 3 && glVerMinor >= 2)) {  // >= gles 3.2
    addGlConstsV3_2(gl);
  }

  numErrors += flushGlErrors(errors);
  return numErrors;
}
}  // namespace

namespace androidgamesdk_deviceinfo {
int createProto(::ProtoInfoWithErrors& proto) {
  int numErrors = 0;

  ProtoInfo& info = *proto.mutable_info();
  info.set_version(1);

  int cpuIndexMax = readCpuIndexMax();
  info.set_cpu_max_index(cpuIndexMax);

  for (int cpuIndex = 0; cpuIndex <= cpuIndexMax; cpuIndex++) {
    ProtoCpuCore* newCore = info.add_cpu_core();
    int64_t freqMax = readCpuFreqMax(cpuIndex);
    if (freqMax > 0) {
      newCore->set_freq_max(freqMax);
    }
  }

  info.set_cpu_present(readCpuPresent());
  info.set_cpu_possible(readCpuPossible());

  ProtoErrors& errors = *proto.mutable_errors();

  std::vector<std::string> hardware;
  numErrors += readHardware(hardware, errors);
  for (const std::string& s : hardware) {
    info.add_hardware(s);
  }

  std::set<std::string> features;
  numErrors += readFeatures(features, errors);
  for (const std::string& s : features) {
    info.add_cpu_extension(s);
  }

  numErrors += addSystemProperties(info, errors);

  int numErrorsEgl = setupEGl(proto);
  numErrors += numErrorsEgl;
  if (numErrorsEgl == 0) {
    numErrors += addGl(proto);
  }

  return numErrors;
}
}  // namespace androidgamesdk_deviceinfo

#include <jni.h>

extern "C" {
JNIEXPORT jbyteArray JNICALL
Java_com_google_androidgamesdk_DeviceInfoJni_getProtoSerialized(
                                        JNIEnv *env, jobject) {
  androidgamesdk_deviceinfo::InfoWithErrors proto;
  androidgamesdk_deviceinfo::createProto(proto);

  size_t bufferSize = proto.ByteSize();
  void* buffer = malloc(bufferSize);
  proto.SerializeToArray(buffer, bufferSize);
  jbyteArray result = env->NewByteArray(bufferSize);
  env->SetByteArrayRegion(result, 0, bufferSize, static_cast<jbyte*>(buffer));
  free(buffer);
  return result;
}
}  // extern "C"
