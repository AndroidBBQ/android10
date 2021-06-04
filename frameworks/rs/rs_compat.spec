AllocationCreateStrided {
    direct
    param RsType vtype
    param RsAllocationMipmapControl mipmaps
    param uint32_t usages
    param uintptr_t ptr
    param size_t requiredAlignment
    ret RsAllocation
}
