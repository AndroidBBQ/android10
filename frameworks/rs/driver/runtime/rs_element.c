#include "rs_core.rsh"
#include "rs_structs.h"

/**
* Element
*/
extern uint32_t __attribute__((overloadable))
        rsElementGetSubElementCount(rs_element e) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL) {
        return 0;
    }
    return element->mHal.state.fieldsCount;
}

extern rs_element __attribute__((overloadable))
        rsElementGetSubElement(rs_element e, uint32_t index) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL || index >= element->mHal.state.fieldsCount) {
        rs_element nullElem = RS_NULL_OBJ;
        return nullElem;
    }
    rs_element returnElem = {
        element->mHal.state.fields[index]
#ifdef __LP64__
        , 0, 0, 0
#endif
    };
    rs_element rs_retval = RS_NULL_OBJ;
    rsSetObject(&rs_retval, returnElem);
    return rs_retval;
}

extern uint32_t __attribute__((overloadable))
        rsElementGetSubElementNameLength(rs_element e, uint32_t index) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL || index >= element->mHal.state.fieldsCount) {
        return 0;
    }
    return element->mHal.state.fieldNameLengths[index];
}

extern uint32_t __attribute__((overloadable))
        rsElementGetSubElementName(rs_element e, uint32_t index, char *name, uint32_t nameLength) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL || index >= element->mHal.state.fieldsCount ||
        nameLength == 0 || name == 0) {
        return 0;
    }

    uint32_t numToCopy = element->mHal.state.fieldNameLengths[index];
    if (nameLength < numToCopy) {
        numToCopy = nameLength;
    }
    // Place the null terminator manually, in case of partial string
    numToCopy --;
    name[numToCopy] = '\0';
    const char *nameSource = element->mHal.state.fieldNames[index];
    for (uint32_t i = 0; i < numToCopy; i ++) {
        name[i] = nameSource[i];
    }
    return numToCopy;
}

extern uint32_t __attribute__((overloadable))
        rsElementGetSubElementArraySize(rs_element e, uint32_t index) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL || index >= element->mHal.state.fieldsCount) {
        return 0;
    }
    return element->mHal.state.fieldArraySizes[index];
}

extern uint32_t __attribute__((overloadable))
        rsElementGetSubElementOffsetBytes(rs_element e, uint32_t index) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL || index >= element->mHal.state.fieldsCount) {
        return 0;
    }
    return element->mHal.state.fieldOffsetBytes[index];
}

extern uint32_t __attribute__((overloadable))
        rsElementGetBytesSize(rs_element e) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL) {
        return 0;
    }
    return element->mHal.state.elementSizeBytes;
}

extern rs_data_type __attribute__((overloadable))
        rsElementGetDataType(rs_element e) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL) {
        return RS_TYPE_INVALID;
    }
    return element->mHal.state.dataType;
}

extern rs_data_kind __attribute__((overloadable))
        rsElementGetDataKind(rs_element e) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL) {
        return RS_KIND_INVALID;
    }
    return element->mHal.state.dataKind;
}

extern uint32_t __attribute__((overloadable))
        rsElementGetVectorSize(rs_element e) {
    Element_t *element = (Element_t *)e.p;
    if (element == NULL) {
        return 0;
    }
    return element->mHal.state.vectorSize;
}
