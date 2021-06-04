/*
 * Copyright (C) 2006 The Android Open Source Project
 *
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

import java.util.HashMap;

public class JType {

    String baseType;
    boolean isArray;
    boolean isClass;
    boolean isString;

    static HashMap<CType,JType> typeMapping = new HashMap<CType,JType>();
    static HashMap<CType,JType> arrayTypeMapping = new HashMap<CType,JType>();

    static {
    // Primitive types
    typeMapping.put(new CType("GLbitfield"), new JType("int"));
    typeMapping.put(new CType("GLboolean"), new JType("boolean"));
    typeMapping.put(new CType("GLclampf"), new JType("float"));
    typeMapping.put(new CType("GLclampx"), new JType("int"));
    typeMapping.put(new CType("GLenum"), new JType("int"));
    typeMapping.put(new CType("GLfloat"), new JType("float"));
    typeMapping.put(new CType("GLfixed"), new JType("int"));
    typeMapping.put(new CType("GLint"), new JType("int"));
    typeMapping.put(new CType("GLintptr"), new JType("int"));
    typeMapping.put(new CType("GLshort"), new JType("short"));
    typeMapping.put(new CType("GLsizei"), new JType("int"));
    typeMapping.put(new CType("GLsizeiptr"), new JType("int"));
    typeMapping.put(new CType("GLubyte"), new JType("byte"));
    typeMapping.put(new CType("GLuint"), new JType("int"));
    typeMapping.put(new CType("void"), new JType("void"));
    typeMapping.put(new CType("GLubyte", true, true), new JType("String", false, false));
    typeMapping.put(new CType("char"), new JType("byte"));
    typeMapping.put(new CType("char", true, true), new JType("String", false, false));
    typeMapping.put(new CType("GLchar", true, true), new JType("String", false, false));
    typeMapping.put(new CType("int"), new JType("int"));
    typeMapping.put(new CType("GLuint64"), new JType("long"));
    typeMapping.put(new CType("GLsync"), new JType("long"));

    // EGL primitive types
    typeMapping.put(new CType("EGLint"), new JType("int"));
    typeMapping.put(new CType("EGLBoolean"), new JType("boolean"));
    typeMapping.put(new CType("EGLenum"), new JType("int"));
    typeMapping.put(new CType("EGLNativePixmapType"), new JType("int"));
    typeMapping.put(new CType("EGLNativeWindowType"), new JType("int"));
    typeMapping.put(new CType("EGLNativeDisplayType"), new JType("long"));
    typeMapping.put(new CType("EGLClientBuffer"), new JType("long"));
    typeMapping.put(new CType("EGLnsecsANDROID"), new JType("long"));
    typeMapping.put(new CType("EGLAttrib"), new JType("long"));
    typeMapping.put(new CType("EGLTime"), new JType("long"));

    // EGL nonprimitive types
    typeMapping.put(new CType("EGLConfig"), new JType("EGLConfig", true, false));
    typeMapping.put(new CType("EGLContext"), new JType("EGLContext", true, false));
    typeMapping.put(new CType("EGLDisplay"), new JType("EGLDisplay", true, false));
    typeMapping.put(new CType("EGLSurface"), new JType("EGLSurface", true, false));
    typeMapping.put(new CType("EGLImage"), new JType("EGLImage", true, false));
    typeMapping.put(new CType("EGLSync"), new JType("EGLSync", true, false));


    // Untyped pointers map to untyped Buffers
    typeMapping.put(new CType("GLvoid", true, true),
            new JType("java.nio.Buffer", true, false));
    typeMapping.put(new CType("GLvoid", false, true),
            new JType("java.nio.Buffer", true, false));
    typeMapping.put(new CType("void", false, true),
            new JType("java.nio.Buffer", true, false));
    typeMapping.put(new CType("void", true, true),
            new JType("java.nio.Buffer", true, false));
    typeMapping.put(new CType("GLeglImageOES", false, false),
            new JType("java.nio.Buffer", true, false));

    // Typed pointers map to typed Buffers
    typeMapping.put(new CType("GLboolean", false, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLenum", false, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLenum", true, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLfixed", false, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLfixed", true, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLfloat", false, true),
            new JType("java.nio.FloatBuffer", true, false));
    typeMapping.put(new CType("GLfloat", true, true),
            new JType("java.nio.FloatBuffer", true, false));
    typeMapping.put(new CType("GLint", false, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLint", true, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLsizei", false, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLuint", false, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLuint", true, true),
            new JType("java.nio.IntBuffer", true, false));
    typeMapping.put(new CType("GLshort", true, true),
            new JType("java.nio.ShortBuffer", true, false));
    typeMapping.put(new CType("GLint64", false, true),
            new JType("java.nio.LongBuffer", true, false));

    // Typed pointers map to arrays + offsets
    arrayTypeMapping.put(new CType("char", false, true),
            new JType("byte", false, true));
    arrayTypeMapping.put(new CType("GLchar", false, true),
            new JType("byte", false, true));
    arrayTypeMapping.put(new CType("GLboolean", false, true),
            new JType("boolean", false, true));
    arrayTypeMapping.put(new CType("GLenum", false, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLenum", true, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLfixed", true, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLfixed", false, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLfloat", false, true), new JType("float", false, true));
    arrayTypeMapping.put(new CType("GLfloat", true, true), new JType("float", false, true));
    arrayTypeMapping.put(new CType("GLint", false, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLint", true, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLshort", true, true), new JType("short", false, true));
    arrayTypeMapping.put(new CType("GLsizei", false, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLsizei", true, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLuint", false, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLuint", true, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLintptr"), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLsizeiptr"), new JType("int", false, true));
    arrayTypeMapping.put(new CType("GLint64", false, true), new JType("long", false, true));

    //EGL typed pointers map to arrays + offsets
    arrayTypeMapping.put(new CType("EGLint", false, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("EGLint", true, true), new JType("int", false, true));
    arrayTypeMapping.put(new CType("EGLConfig", false, true), new JType("EGLConfig", true, true));
    arrayTypeMapping.put(new CType("EGLConfig", true, true), new JType("EGLConfig", true, true));
    arrayTypeMapping.put(new CType("EGLAttrib", false, true), new JType("long", false, true));
    arrayTypeMapping.put(new CType("EGLAttrib", true, true), new JType("long", false, true));

    }

    public JType() {
    }

    public JType(String primitiveTypeName) {
    this.baseType = primitiveTypeName;
    this.isClass = false;
    this.isArray = false;
    }

    public JType(String primitiveTypeName, boolean isClass, boolean isArray) {
    this.baseType = primitiveTypeName;
    this.isClass = isClass;
    this.isArray = isArray;
    }

    public String getBaseType() {
    return baseType;
    }

    @Override
    public String toString() {
    return baseType + (isArray ? "[]" : "");
    }

    public boolean isArray() {
    return isArray;
    }

    public boolean isClass() {
    return isClass;
    }

    public boolean isString() {
        return baseType.equals("String");
    }

    public boolean isPrimitive() {
    return !isClass() && !isArray();
    }

    public boolean isVoid() {
    return baseType.equals("void");
    }

    public boolean isBuffer() {
    return baseType.indexOf("Buffer") != -1;
    }

    public boolean isTypedBuffer() {
    return !baseType.equals("java.nio.Buffer") &&
        (baseType.indexOf("Buffer") != -1);
    }

    public JType getArrayTypeForTypedBuffer() {
      if (!isTypedBuffer()) {
          throw new RuntimeException("Not typed buffer type " + this);
      }
      switch (baseType) {
        case "java.nio.ByteBuffer":
          return new JType("byte", false, true);
        case "java.nio.BooleanBuffer":
          return new JType("boolean", false, true);
        case "java.nio.ShortBuffer":
          return new JType("short", false, true);
        case "java.nio.CharBuffer":
          return new JType("char", false, true);
        case "java.nio.IntBuffer":
          return new JType("int", false, true);
        case "java.nio.LongBuffer":
          return new JType("long", false, true);
        case "java.nio.FloatBuffer":
          return new JType("float", false, true);
        case "java.nio.DoubleBuffer":
          return new JType("double", false, true);
        default:
          throw new RuntimeException("Unknown typed buffer type " + this);
      }
    }

    public String getArrayGetterForPrimitiveArray() {
      if (!isArray() || isClass()) {
          throw new RuntimeException("Not array type " + this);
      }
      switch (baseType) {
        case "byte":
          return "GetByteArrayElements";
        case "boolean":
          return "GetBooleanArrayElements";
        case "short":
          return "GetShortArrayElements";
        case "char":
          return "GetCharArrayElements";
        case "int":
          return "GetIntArrayElements";
        case "long":
          return "GetLongArrayElements";
        case "float":
          return "GetFloatArrayElements";
        case "double":
          return "GetDoubleArrayElements";
        default:
          throw new RuntimeException("Unknown array type " + this);
      }
    }

    public String getArrayReleaserForPrimitiveArray() {
      if (!isArray() || isClass()) {
          throw new RuntimeException("Not array type " + this);
      }
      switch (baseType) {
        case "byte":
          return "ReleaseByteArrayElements";
        case "boolean":
          return "ReleaseBooleanArrayElements";
        case "short":
          return "ReleaseShortArrayElements";
        case "char":
          return "ReleaseCharArrayElements";
        case "int":
          return "ReleaseIntArrayElements";
        case "long":
          return "ReleaseLongArrayElements";
        case "float":
          return "ReleaseFloatArrayElements";
        case "double":
          return "ReleaseDoubleArrayElements";
        default:
          throw new RuntimeException("Unknown array type " + this);
      }
    }

    public boolean isEGLHandle() {
    return !isPrimitive() &&
        (baseType.startsWith("EGL"));
    }

    public static JType convert(CType ctype, boolean useArray) {
     JType javaType = null;
     if (useArray) {
         javaType = arrayTypeMapping.get(ctype);
     }
     if (javaType == null) {
         javaType = typeMapping.get(ctype);
     }
     if (javaType == null) {
         throw new RuntimeException("Unsupported C type: " + ctype);
     }
     return javaType;
    }
}
