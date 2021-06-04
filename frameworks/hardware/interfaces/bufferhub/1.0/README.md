# Why is this marked as '.hidl_for_test'?

This is used to explicitly exclude the interface from the VNDK. Disallow direct vendor access
as this interface should only be used by the Android platform. Vendors should use
libnativewindow ll-ndk API to access BufferHub.
