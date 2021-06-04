/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.usb;
/** @hide */
public interface IUsbManager extends android.os.IInterface
{
  /** Default implementation for IUsbManager. */
  public static class Default implements android.hardware.usb.IUsbManager
  {
    /* Returns a list of all currently attached USB devices */
    @Override public void getDeviceList(android.os.Bundle devices) throws android.os.RemoteException
    {
    }
    /* Returns a file descriptor for communicating with the USB device.
         * The native fd can be passed to usb_device_new() in libusbhost.
         */
    @Override public android.os.ParcelFileDescriptor openDevice(java.lang.String deviceName, java.lang.String packageName) throws android.os.RemoteException
    {
      return null;
    }
    /* Returns the currently attached USB accessory */
    @Override public android.hardware.usb.UsbAccessory getCurrentAccessory() throws android.os.RemoteException
    {
      return null;
    }
    /* Returns a file descriptor for communicating with the USB accessory.
         * This file descriptor can be used with standard Java file operations.
         */
    @Override public android.os.ParcelFileDescriptor openAccessory(android.hardware.usb.UsbAccessory accessory) throws android.os.RemoteException
    {
      return null;
    }
    /* Sets the default package for a USB device
         * (or clears it if the package name is null)
         */
    @Override public void setDevicePackage(android.hardware.usb.UsbDevice device, java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    /* Sets the default package for a USB accessory
         * (or clears it if the package name is null)
         */
    @Override public void setAccessoryPackage(android.hardware.usb.UsbAccessory accessory, java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    /* Returns true if the caller has permission to access the device. */
    @Override public boolean hasDevicePermission(android.hardware.usb.UsbDevice device, java.lang.String packageName) throws android.os.RemoteException
    {
      return false;
    }
    /* Returns true if the caller has permission to access the accessory. */
    @Override public boolean hasAccessoryPermission(android.hardware.usb.UsbAccessory accessory) throws android.os.RemoteException
    {
      return false;
    }
    /* Requests permission for the given package to access the device.
         * Will display a system dialog to query the user if permission
         * had not already been given.
         */
    @Override public void requestDevicePermission(android.hardware.usb.UsbDevice device, java.lang.String packageName, android.app.PendingIntent pi) throws android.os.RemoteException
    {
    }
    /* Requests permission for the given package to access the accessory.
         * Will display a system dialog to query the user if permission
         * had not already been given. Result is returned via pi.
         */
    @Override public void requestAccessoryPermission(android.hardware.usb.UsbAccessory accessory, java.lang.String packageName, android.app.PendingIntent pi) throws android.os.RemoteException
    {
    }
    /* Grants permission for the given UID to access the device */
    @Override public void grantDevicePermission(android.hardware.usb.UsbDevice device, int uid) throws android.os.RemoteException
    {
    }
    /* Grants permission for the given UID to access the accessory */
    @Override public void grantAccessoryPermission(android.hardware.usb.UsbAccessory accessory, int uid) throws android.os.RemoteException
    {
    }
    /* Returns true if the USB manager has default preferences or permissions for the package */
    @Override public boolean hasDefaults(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
      return false;
    }
    /* Clears default preferences and permissions for the package */
    @Override public void clearDefaults(java.lang.String packageName, int userId) throws android.os.RemoteException
    {
    }
    /* Returns true if the specified USB function is enabled. */
    @Override public boolean isFunctionEnabled(java.lang.String function) throws android.os.RemoteException
    {
      return false;
    }
    /* Sets the current USB function. */
    @Override public void setCurrentFunctions(long functions) throws android.os.RemoteException
    {
    }
    /* Compatibility version of setCurrentFunctions(long). */
    @Override public void setCurrentFunction(java.lang.String function, boolean usbDataUnlocked) throws android.os.RemoteException
    {
    }
    /* Gets the current USB functions. */
    @Override public long getCurrentFunctions() throws android.os.RemoteException
    {
      return 0L;
    }
    /* Sets the screen unlocked USB function(s), which will be set automatically
         * when the screen is unlocked.
         */
    @Override public void setScreenUnlockedFunctions(long functions) throws android.os.RemoteException
    {
    }
    /* Gets the current screen unlocked functions. */
    @Override public long getScreenUnlockedFunctions() throws android.os.RemoteException
    {
      return 0L;
    }
    /* Get the functionfs control handle for the given function. Usb
         * descriptors will already be written, and the handle will be
         * ready to use.
         */
    @Override public android.os.ParcelFileDescriptor getControlFd(long function) throws android.os.RemoteException
    {
      return null;
    }
    /* Gets the list of USB ports. */
    @Override public java.util.List<android.hardware.usb.ParcelableUsbPort> getPorts() throws android.os.RemoteException
    {
      return null;
    }
    /* Gets the status of the specified USB port. */
    @Override public android.hardware.usb.UsbPortStatus getPortStatus(java.lang.String portId) throws android.os.RemoteException
    {
      return null;
    }
    /* Sets the port's current role. */
    @Override public void setPortRoles(java.lang.String portId, int powerRole, int dataRole) throws android.os.RemoteException
    {
    }
    /* Enable/disable contaminant detection */
    @Override public void enableContaminantDetection(java.lang.String portId, boolean enable) throws android.os.RemoteException
    {
    }
    /* Sets USB device connection handler. */
    @Override public void setUsbDeviceConnectionHandler(android.content.ComponentName usbDeviceConnectionHandler) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.usb.IUsbManager
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.usb.IUsbManager";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.usb.IUsbManager interface,
     * generating a proxy if needed.
     */
    public static android.hardware.usb.IUsbManager asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.usb.IUsbManager))) {
        return ((android.hardware.usb.IUsbManager)iin);
      }
      return new android.hardware.usb.IUsbManager.Stub.Proxy(obj);
    }
    @Override public android.os.IBinder asBinder()
    {
      return this;
    }
    @Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
    {
      java.lang.String descriptor = DESCRIPTOR;
      switch (code)
      {
        case INTERFACE_TRANSACTION:
        {
          reply.writeString(descriptor);
          return true;
        }
        case TRANSACTION_getDeviceList:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          _arg0 = new android.os.Bundle();
          this.getDeviceList(_arg0);
          reply.writeNoException();
          if ((_arg0!=null)) {
            reply.writeInt(1);
            _arg0.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_openDevice:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.os.ParcelFileDescriptor _result = this.openDevice(_arg0, _arg1);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getCurrentAccessory:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbAccessory _result = this.getCurrentAccessory();
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_openAccessory:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbAccessory _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbAccessory.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.ParcelFileDescriptor _result = this.openAccessory(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_setDevicePackage:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.setDevicePackage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setAccessoryPackage:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbAccessory _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbAccessory.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _arg2;
          _arg2 = data.readInt();
          this.setAccessoryPackage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasDevicePermission:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          boolean _result = this.hasDevicePermission(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_hasAccessoryPermission:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbAccessory _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbAccessory.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _result = this.hasAccessoryPermission(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_requestDevicePermission:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.app.PendingIntent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.requestDevicePermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_requestAccessoryPermission:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbAccessory _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbAccessory.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.app.PendingIntent _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.requestAccessoryPermission(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_grantDevicePermission:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbDevice _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbDevice.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.grantDevicePermission(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_grantAccessoryPermission:
        {
          data.enforceInterface(descriptor);
          android.hardware.usb.UsbAccessory _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.hardware.usb.UsbAccessory.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.grantAccessoryPermission(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_hasDefaults:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          boolean _result = this.hasDefaults(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_clearDefaults:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          this.clearDefaults(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_isFunctionEnabled:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _result = this.isFunctionEnabled(_arg0);
          reply.writeNoException();
          reply.writeInt(((_result)?(1):(0)));
          return true;
        }
        case TRANSACTION_setCurrentFunctions:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setCurrentFunctions(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setCurrentFunction:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.setCurrentFunction(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getCurrentFunctions:
        {
          data.enforceInterface(descriptor);
          long _result = this.getCurrentFunctions();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_setScreenUnlockedFunctions:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          this.setScreenUnlockedFunctions(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getScreenUnlockedFunctions:
        {
          data.enforceInterface(descriptor);
          long _result = this.getScreenUnlockedFunctions();
          reply.writeNoException();
          reply.writeLong(_result);
          return true;
        }
        case TRANSACTION_getControlFd:
        {
          data.enforceInterface(descriptor);
          long _arg0;
          _arg0 = data.readLong();
          android.os.ParcelFileDescriptor _result = this.getControlFd(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_getPorts:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.usb.ParcelableUsbPort> _result = this.getPorts();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_getPortStatus:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          android.hardware.usb.UsbPortStatus _result = this.getPortStatus(_arg0);
          reply.writeNoException();
          if ((_result!=null)) {
            reply.writeInt(1);
            _result.writeToParcel(reply, android.os.Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
          }
          else {
            reply.writeInt(0);
          }
          return true;
        }
        case TRANSACTION_setPortRoles:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.setPortRoles(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableContaminantDetection:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          this.enableContaminantDetection(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_setUsbDeviceConnectionHandler:
        {
          data.enforceInterface(descriptor);
          android.content.ComponentName _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.content.ComponentName.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.setUsbDeviceConnectionHandler(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.usb.IUsbManager
    {
      private android.os.IBinder mRemote;
      Proxy(android.os.IBinder remote)
      {
        mRemote = remote;
      }
      @Override public android.os.IBinder asBinder()
      {
        return mRemote;
      }
      public java.lang.String getInterfaceDescriptor()
      {
        return DESCRIPTOR;
      }
      /* Returns a list of all currently attached USB devices */
      @Override public void getDeviceList(android.os.Bundle devices) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getDeviceList, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().getDeviceList(devices);
            return;
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            devices.readFromParcel(_reply);
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Returns a file descriptor for communicating with the USB device.
           * The native fd can be passed to usb_device_new() in libusbhost.
           */
      @Override public android.os.ParcelFileDescriptor openDevice(java.lang.String deviceName, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(deviceName);
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openDevice, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openDevice(deviceName, packageName);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Returns the currently attached USB accessory */
      @Override public android.hardware.usb.UsbAccessory getCurrentAccessory() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.usb.UsbAccessory _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentAccessory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentAccessory();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.usb.UsbAccessory.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Returns a file descriptor for communicating with the USB accessory.
           * This file descriptor can be used with standard Java file operations.
           */
      @Override public android.os.ParcelFileDescriptor openAccessory(android.hardware.usb.UsbAccessory accessory) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accessory!=null)) {
            _data.writeInt(1);
            accessory.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_openAccessory, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openAccessory(accessory);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Sets the default package for a USB device
           * (or clears it if the package name is null)
           */
      @Override public void setDevicePackage(android.hardware.usb.UsbDevice device, java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setDevicePackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setDevicePackage(device, packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Sets the default package for a USB accessory
           * (or clears it if the package name is null)
           */
      @Override public void setAccessoryPackage(android.hardware.usb.UsbAccessory accessory, java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accessory!=null)) {
            _data.writeInt(1);
            accessory.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setAccessoryPackage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setAccessoryPackage(accessory, packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Returns true if the caller has permission to access the device. */
      @Override public boolean hasDevicePermission(android.hardware.usb.UsbDevice device, java.lang.String packageName) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasDevicePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasDevicePermission(device, packageName);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Returns true if the caller has permission to access the accessory. */
      @Override public boolean hasAccessoryPermission(android.hardware.usb.UsbAccessory accessory) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accessory!=null)) {
            _data.writeInt(1);
            accessory.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasAccessoryPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasAccessoryPermission(accessory);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Requests permission for the given package to access the device.
           * Will display a system dialog to query the user if permission
           * had not already been given.
           */
      @Override public void requestDevicePermission(android.hardware.usb.UsbDevice device, java.lang.String packageName, android.app.PendingIntent pi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          if ((pi!=null)) {
            _data.writeInt(1);
            pi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestDevicePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestDevicePermission(device, packageName, pi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Requests permission for the given package to access the accessory.
           * Will display a system dialog to query the user if permission
           * had not already been given. Result is returned via pi.
           */
      @Override public void requestAccessoryPermission(android.hardware.usb.UsbAccessory accessory, java.lang.String packageName, android.app.PendingIntent pi) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accessory!=null)) {
            _data.writeInt(1);
            accessory.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(packageName);
          if ((pi!=null)) {
            _data.writeInt(1);
            pi.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_requestAccessoryPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().requestAccessoryPermission(accessory, packageName, pi);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Grants permission for the given UID to access the device */
      @Override public void grantDevicePermission(android.hardware.usb.UsbDevice device, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((device!=null)) {
            _data.writeInt(1);
            device.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantDevicePermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantDevicePermission(device, uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Grants permission for the given UID to access the accessory */
      @Override public void grantAccessoryPermission(android.hardware.usb.UsbAccessory accessory, int uid) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((accessory!=null)) {
            _data.writeInt(1);
            accessory.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(uid);
          boolean _status = mRemote.transact(Stub.TRANSACTION_grantAccessoryPermission, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().grantAccessoryPermission(accessory, uid);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Returns true if the USB manager has default preferences or permissions for the package */
      @Override public boolean hasDefaults(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_hasDefaults, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().hasDefaults(packageName, userId);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Clears default preferences and permissions for the package */
      @Override public void clearDefaults(java.lang.String packageName, int userId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(packageName);
          _data.writeInt(userId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_clearDefaults, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().clearDefaults(packageName, userId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Returns true if the specified USB function is enabled. */
      @Override public boolean isFunctionEnabled(java.lang.String function) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        boolean _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(function);
          boolean _status = mRemote.transact(Stub.TRANSACTION_isFunctionEnabled, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().isFunctionEnabled(function);
          }
          _reply.readException();
          _result = (0!=_reply.readInt());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Sets the current USB function. */
      @Override public void setCurrentFunctions(long functions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(functions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCurrentFunctions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCurrentFunctions(functions);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Compatibility version of setCurrentFunctions(long). */
      @Override public void setCurrentFunction(java.lang.String function, boolean usbDataUnlocked) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(function);
          _data.writeInt(((usbDataUnlocked)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setCurrentFunction, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setCurrentFunction(function, usbDataUnlocked);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Gets the current USB functions. */
      @Override public long getCurrentFunctions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getCurrentFunctions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getCurrentFunctions();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Sets the screen unlocked USB function(s), which will be set automatically
           * when the screen is unlocked.
           */
      @Override public void setScreenUnlockedFunctions(long functions) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(functions);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setScreenUnlockedFunctions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setScreenUnlockedFunctions(functions);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Gets the current screen unlocked functions. */
      @Override public long getScreenUnlockedFunctions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        long _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getScreenUnlockedFunctions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getScreenUnlockedFunctions();
          }
          _reply.readException();
          _result = _reply.readLong();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Get the functionfs control handle for the given function. Usb
           * descriptors will already be written, and the handle will be
           * ready to use.
           */
      @Override public android.os.ParcelFileDescriptor getControlFd(long function) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.os.ParcelFileDescriptor _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeLong(function);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getControlFd, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getControlFd(function);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Gets the list of USB ports. */
      @Override public java.util.List<android.hardware.usb.ParcelableUsbPort> getPorts() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.usb.ParcelableUsbPort> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPorts, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPorts();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.usb.ParcelableUsbPort.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Gets the status of the specified USB port. */
      @Override public android.hardware.usb.UsbPortStatus getPortStatus(java.lang.String portId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.usb.UsbPortStatus _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(portId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPortStatus, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPortStatus(portId);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.usb.UsbPortStatus.CREATOR.createFromParcel(_reply);
          }
          else {
            _result = null;
          }
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      /* Sets the port's current role. */
      @Override public void setPortRoles(java.lang.String portId, int powerRole, int dataRole) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(portId);
          _data.writeInt(powerRole);
          _data.writeInt(dataRole);
          boolean _status = mRemote.transact(Stub.TRANSACTION_setPortRoles, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setPortRoles(portId, powerRole, dataRole);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Enable/disable contaminant detection */
      @Override public void enableContaminantDetection(java.lang.String portId, boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(portId);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableContaminantDetection, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableContaminantDetection(portId, enable);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /* Sets USB device connection handler. */
      @Override public void setUsbDeviceConnectionHandler(android.content.ComponentName usbDeviceConnectionHandler) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((usbDeviceConnectionHandler!=null)) {
            _data.writeInt(1);
            usbDeviceConnectionHandler.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setUsbDeviceConnectionHandler, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setUsbDeviceConnectionHandler(usbDeviceConnectionHandler);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.usb.IUsbManager sDefaultImpl;
    }
    static final int TRANSACTION_getDeviceList = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_openDevice = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getCurrentAccessory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_openAccessory = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_setDevicePackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_setAccessoryPackage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_hasDevicePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_hasAccessoryPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_requestDevicePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_requestAccessoryPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_grantDevicePermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_grantAccessoryPermission = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_hasDefaults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_clearDefaults = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_isFunctionEnabled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_setCurrentFunctions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setCurrentFunction = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_getCurrentFunctions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_setScreenUnlockedFunctions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    static final int TRANSACTION_getScreenUnlockedFunctions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 19);
    static final int TRANSACTION_getControlFd = (android.os.IBinder.FIRST_CALL_TRANSACTION + 20);
    static final int TRANSACTION_getPorts = (android.os.IBinder.FIRST_CALL_TRANSACTION + 21);
    static final int TRANSACTION_getPortStatus = (android.os.IBinder.FIRST_CALL_TRANSACTION + 22);
    static final int TRANSACTION_setPortRoles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 23);
    static final int TRANSACTION_enableContaminantDetection = (android.os.IBinder.FIRST_CALL_TRANSACTION + 24);
    static final int TRANSACTION_setUsbDeviceConnectionHandler = (android.os.IBinder.FIRST_CALL_TRANSACTION + 25);
    public static boolean setDefaultImpl(android.hardware.usb.IUsbManager impl) {
      // Only one user of this interface can use this function
      // at a time. This is a heuristic to detect if two different
      // users in the same process use this function.
      if (Stub.Proxy.sDefaultImpl != null) {
        throw new IllegalStateException("setDefaultImpl() called twice");
      }
      if (impl != null) {
        Stub.Proxy.sDefaultImpl = impl;
        return true;
      }
      return false;
    }
    public static android.hardware.usb.IUsbManager getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /* Returns a list of all currently attached USB devices */
  public void getDeviceList(android.os.Bundle devices) throws android.os.RemoteException;
  /* Returns a file descriptor for communicating with the USB device.
       * The native fd can be passed to usb_device_new() in libusbhost.
       */
  public android.os.ParcelFileDescriptor openDevice(java.lang.String deviceName, java.lang.String packageName) throws android.os.RemoteException;
  /* Returns the currently attached USB accessory */
  public android.hardware.usb.UsbAccessory getCurrentAccessory() throws android.os.RemoteException;
  /* Returns a file descriptor for communicating with the USB accessory.
       * This file descriptor can be used with standard Java file operations.
       */
  public android.os.ParcelFileDescriptor openAccessory(android.hardware.usb.UsbAccessory accessory) throws android.os.RemoteException;
  /* Sets the default package for a USB device
       * (or clears it if the package name is null)
       */
  public void setDevicePackage(android.hardware.usb.UsbDevice device, java.lang.String packageName, int userId) throws android.os.RemoteException;
  /* Sets the default package for a USB accessory
       * (or clears it if the package name is null)
       */
  public void setAccessoryPackage(android.hardware.usb.UsbAccessory accessory, java.lang.String packageName, int userId) throws android.os.RemoteException;
  /* Returns true if the caller has permission to access the device. */
  public boolean hasDevicePermission(android.hardware.usb.UsbDevice device, java.lang.String packageName) throws android.os.RemoteException;
  /* Returns true if the caller has permission to access the accessory. */
  public boolean hasAccessoryPermission(android.hardware.usb.UsbAccessory accessory) throws android.os.RemoteException;
  /* Requests permission for the given package to access the device.
       * Will display a system dialog to query the user if permission
       * had not already been given.
       */
  public void requestDevicePermission(android.hardware.usb.UsbDevice device, java.lang.String packageName, android.app.PendingIntent pi) throws android.os.RemoteException;
  /* Requests permission for the given package to access the accessory.
       * Will display a system dialog to query the user if permission
       * had not already been given. Result is returned via pi.
       */
  public void requestAccessoryPermission(android.hardware.usb.UsbAccessory accessory, java.lang.String packageName, android.app.PendingIntent pi) throws android.os.RemoteException;
  /* Grants permission for the given UID to access the device */
  public void grantDevicePermission(android.hardware.usb.UsbDevice device, int uid) throws android.os.RemoteException;
  /* Grants permission for the given UID to access the accessory */
  public void grantAccessoryPermission(android.hardware.usb.UsbAccessory accessory, int uid) throws android.os.RemoteException;
  /* Returns true if the USB manager has default preferences or permissions for the package */
  public boolean hasDefaults(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /* Clears default preferences and permissions for the package */
  public void clearDefaults(java.lang.String packageName, int userId) throws android.os.RemoteException;
  /* Returns true if the specified USB function is enabled. */
  public boolean isFunctionEnabled(java.lang.String function) throws android.os.RemoteException;
  /* Sets the current USB function. */
  public void setCurrentFunctions(long functions) throws android.os.RemoteException;
  /* Compatibility version of setCurrentFunctions(long). */
  public void setCurrentFunction(java.lang.String function, boolean usbDataUnlocked) throws android.os.RemoteException;
  /* Gets the current USB functions. */
  public long getCurrentFunctions() throws android.os.RemoteException;
  /* Sets the screen unlocked USB function(s), which will be set automatically
       * when the screen is unlocked.
       */
  public void setScreenUnlockedFunctions(long functions) throws android.os.RemoteException;
  /* Gets the current screen unlocked functions. */
  public long getScreenUnlockedFunctions() throws android.os.RemoteException;
  /* Get the functionfs control handle for the given function. Usb
       * descriptors will already be written, and the handle will be
       * ready to use.
       */
  public android.os.ParcelFileDescriptor getControlFd(long function) throws android.os.RemoteException;
  /* Gets the list of USB ports. */
  public java.util.List<android.hardware.usb.ParcelableUsbPort> getPorts() throws android.os.RemoteException;
  /* Gets the status of the specified USB port. */
  public android.hardware.usb.UsbPortStatus getPortStatus(java.lang.String portId) throws android.os.RemoteException;
  /* Sets the port's current role. */
  public void setPortRoles(java.lang.String portId, int powerRole, int dataRole) throws android.os.RemoteException;
  /* Enable/disable contaminant detection */
  public void enableContaminantDetection(java.lang.String portId, boolean enable) throws android.os.RemoteException;
  /* Sets USB device connection handler. */
  public void setUsbDeviceConnectionHandler(android.content.ComponentName usbDeviceConnectionHandler) throws android.os.RemoteException;
}
