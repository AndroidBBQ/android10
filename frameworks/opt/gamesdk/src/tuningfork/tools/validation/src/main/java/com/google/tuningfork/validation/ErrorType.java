/*
 * Copyright (C) 2019 The Android Open Source Project
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
 * limitations under the License
 */

package com.google.tuningfork.validation;

/** Validation errors */
public enum ErrorType {
  // Annotation field is empty
  ANNOTATION_EMPTY(ErrorGroup.ANNOTATION),
  // Annotation field is too complex - contains oneofs/nestedtypes/extensions
  ANNOTATION_COMPLEX(ErrorGroup.ANNOTATION),
  // Annotation must contains enums only
  ANNOTATION_TYPE(ErrorGroup.ANNOTATION),
  // FidelityParams fied is empty
  FIDELITY_PARAMS_EMPTY(ErrorGroup.FIDELITY),
  // FidelityParams field is complex - contains  oneof/nestedtypes/extensions
  FIDELITY_PARAMS_COMPLEX(ErrorGroup.FIDELITY),
  // FidelityParams can only contains float, int32 or enum
  FIDELITY_PARAMS_TYPE(ErrorGroup.FIDELITY),
  // Fidelity parameters are empty
  DEV_FIDELITY_PARAMETERS_EMPTY(ErrorGroup.DEV_FIDELITY),
  // Fidelity parameters parsing error
  DEV_FIDELITY_PARAMETERS_PARSING(ErrorGroup.DEV_FIDELITY),
  // Fidelity parameters encoding textproto file
  DEV_FIDELITY_PARAMETERS_ENCODING(ErrorGroup.DEV_FIDELITY),
  // Fidelity parameters reading file
  DEV_FIDELITY_PARAMETERS_READING(ErrorGroup.DEV_FIDELITY),
  // Parsing error
  SETTINGS_PARSING(ErrorGroup.SETTINGS),
  // Histogram field is empty
  HISTOGRAM_EMPTY(ErrorGroup.SETTINGS),
  // Aggreagtion field is empty
  AGGREGATION_EMPTY(ErrorGroup.SETTINGS),
  // Aggregation contains incorrect  max_instrumentation_keys field
  AGGREGATION_INSTRUMENTATION_KEY(ErrorGroup.SETTINGS),
  // Aggregation contains incorrect annotation_enum_sizes
  AGGREGATION_ANNOTATIONS(ErrorGroup.SETTINGS);

  private final ErrorGroup group;

  public ErrorGroup getGroup() {
    return group;
  }

  ErrorType(ErrorGroup group) {
    this.group = group;
  }

  /** Validation group of errors */
  public enum ErrorGroup {
    ANNOTATION,
    FIDELITY,
    DEV_FIDELITY,
    SETTINGS,
  }
};
