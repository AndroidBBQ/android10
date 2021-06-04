/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
/**
 * @hide
 */
public interface IIpSecService extends android.os.IInterface
{
  /** Default implementation for IIpSecService. */
  public static class Default implements android.net.IIpSecService
  {
    @Override public android.net.IpSecSpiResponse allocateSecurityParameterIndex(java.lang.String destinationAddress, int requestedSpi, android.os.IBinder binder) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void releaseSecurityParameterIndex(int resourceId) throws android.os.RemoteException
    {
    }
    @Override public android.net.IpSecUdpEncapResponse openUdpEncapsulationSocket(int port, android.os.IBinder binder) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void closeUdpEncapsulationSocket(int resourceId) throws android.os.RemoteException
    {
    }
    @Override public android.net.IpSecTunnelInterfaceResponse createTunnelInterface(java.lang.String localAddr, java.lang.String remoteAddr, android.net.Network underlyingNetwork, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void addAddressToTunnelInterface(int tunnelResourceId, android.net.LinkAddress localAddr, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void removeAddressFromTunnelInterface(int tunnelResourceId, android.net.LinkAddress localAddr, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void deleteTunnelInterface(int resourceId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public android.net.IpSecTransformResponse createTransform(android.net.IpSecConfig c, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException
    {
      return null;
    }
    @Override public void deleteTransform(int transformId) throws android.os.RemoteException
    {
    }
    @Override public void applyTransportModeTransform(android.os.ParcelFileDescriptor socket, int direction, int transformId) throws android.os.RemoteException
    {
    }
    @Override public void applyTunnelModeTransform(int tunnelResourceId, int direction, int transformResourceId, java.lang.String callingPackage) throws android.os.RemoteException
    {
    }
    @Override public void removeTransportModeTransforms(android.os.ParcelFileDescriptor socket) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.net.IIpSecService
  {
    private static final java.lang.String DESCRIPTOR = "android.net.IIpSecService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.net.IIpSecService interface,
     * generating a proxy if needed.
     */
    public static android.net.IIpSecService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.net.IIpSecService))) {
        return ((android.net.IIpSecService)iin);
      }
      return new android.net.IIpSecService.Stub.Proxy(obj);
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
        case TRANSACTION_allocateSecurityParameterIndex:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          int _arg1;
          _arg1 = data.readInt();
          android.os.IBinder _arg2;
          _arg2 = data.readStrongBinder();
          android.net.IpSecSpiResponse _result = this.allocateSecurityParameterIndex(_arg0, _arg1, _arg2);
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
        case TRANSACTION_releaseSecurityParameterIndex:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.releaseSecurityParameterIndex(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_openUdpEncapsulationSocket:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          android.net.IpSecUdpEncapResponse _result = this.openUdpEncapsulationSocket(_arg0, _arg1);
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
        case TRANSACTION_closeUdpEncapsulationSocket:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.closeUdpEncapsulationSocket(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createTunnelInterface:
        {
          data.enforceInterface(descriptor);
          java.lang.String _arg0;
          _arg0 = data.readString();
          java.lang.String _arg1;
          _arg1 = data.readString();
          android.net.Network _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.net.Network.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          android.os.IBinder _arg3;
          _arg3 = data.readStrongBinder();
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.net.IpSecTunnelInterfaceResponse _result = this.createTunnelInterface(_arg0, _arg1, _arg2, _arg3, _arg4);
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
        case TRANSACTION_addAddressToTunnelInterface:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.LinkAddress _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.LinkAddress.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.addAddressToTunnelInterface(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeAddressFromTunnelInterface:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.net.LinkAddress _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.net.LinkAddress.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          java.lang.String _arg2;
          _arg2 = data.readString();
          this.removeAddressFromTunnelInterface(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_deleteTunnelInterface:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          this.deleteTunnelInterface(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_createTransform:
        {
          data.enforceInterface(descriptor);
          android.net.IpSecConfig _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.net.IpSecConfig.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          android.os.IBinder _arg1;
          _arg1 = data.readStrongBinder();
          java.lang.String _arg2;
          _arg2 = data.readString();
          android.net.IpSecTransformResponse _result = this.createTransform(_arg0, _arg1, _arg2);
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
        case TRANSACTION_deleteTransform:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.deleteTransform(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_applyTransportModeTransform:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          this.applyTransportModeTransform(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_applyTunnelModeTransform:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _arg2;
          _arg2 = data.readInt();
          java.lang.String _arg3;
          _arg3 = data.readString();
          this.applyTunnelModeTransform(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeTransportModeTransforms:
        {
          data.enforceInterface(descriptor);
          android.os.ParcelFileDescriptor _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.ParcelFileDescriptor.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.removeTransportModeTransforms(_arg0);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.net.IIpSecService
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
      @Override public android.net.IpSecSpiResponse allocateSecurityParameterIndex(java.lang.String destinationAddress, int requestedSpi, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.IpSecSpiResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(destinationAddress);
          _data.writeInt(requestedSpi);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_allocateSecurityParameterIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().allocateSecurityParameterIndex(destinationAddress, requestedSpi, binder);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.IpSecSpiResponse.CREATOR.createFromParcel(_reply);
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
      @Override public void releaseSecurityParameterIndex(int resourceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(resourceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_releaseSecurityParameterIndex, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().releaseSecurityParameterIndex(resourceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.IpSecUdpEncapResponse openUdpEncapsulationSocket(int port, android.os.IBinder binder) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.IpSecUdpEncapResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(port);
          _data.writeStrongBinder(binder);
          boolean _status = mRemote.transact(Stub.TRANSACTION_openUdpEncapsulationSocket, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().openUdpEncapsulationSocket(port, binder);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.IpSecUdpEncapResponse.CREATOR.createFromParcel(_reply);
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
      @Override public void closeUdpEncapsulationSocket(int resourceId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(resourceId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_closeUdpEncapsulationSocket, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().closeUdpEncapsulationSocket(resourceId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.IpSecTunnelInterfaceResponse createTunnelInterface(java.lang.String localAddr, java.lang.String remoteAddr, android.net.Network underlyingNetwork, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.IpSecTunnelInterfaceResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeString(localAddr);
          _data.writeString(remoteAddr);
          if ((underlyingNetwork!=null)) {
            _data.writeInt(1);
            underlyingNetwork.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(binder);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createTunnelInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createTunnelInterface(localAddr, remoteAddr, underlyingNetwork, binder, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.IpSecTunnelInterfaceResponse.CREATOR.createFromParcel(_reply);
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
      @Override public void addAddressToTunnelInterface(int tunnelResourceId, android.net.LinkAddress localAddr, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tunnelResourceId);
          if ((localAddr!=null)) {
            _data.writeInt(1);
            localAddr.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addAddressToTunnelInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addAddressToTunnelInterface(tunnelResourceId, localAddr, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeAddressFromTunnelInterface(int tunnelResourceId, android.net.LinkAddress localAddr, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tunnelResourceId);
          if ((localAddr!=null)) {
            _data.writeInt(1);
            localAddr.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeAddressFromTunnelInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeAddressFromTunnelInterface(tunnelResourceId, localAddr, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void deleteTunnelInterface(int resourceId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(resourceId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteTunnelInterface, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteTunnelInterface(resourceId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public android.net.IpSecTransformResponse createTransform(android.net.IpSecConfig c, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.net.IpSecTransformResponse _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((c!=null)) {
            _data.writeInt(1);
            c.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeStrongBinder(binder);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createTransform, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createTransform(c, binder, callingPackage);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.net.IpSecTransformResponse.CREATOR.createFromParcel(_reply);
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
      @Override public void deleteTransform(int transformId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(transformId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_deleteTransform, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().deleteTransform(transformId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void applyTransportModeTransform(android.os.ParcelFileDescriptor socket, int direction, int transformId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((socket!=null)) {
            _data.writeInt(1);
            socket.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(direction);
          _data.writeInt(transformId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyTransportModeTransform, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyTransportModeTransform(socket, direction, transformId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void applyTunnelModeTransform(int tunnelResourceId, int direction, int transformResourceId, java.lang.String callingPackage) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(tunnelResourceId);
          _data.writeInt(direction);
          _data.writeInt(transformResourceId);
          _data.writeString(callingPackage);
          boolean _status = mRemote.transact(Stub.TRANSACTION_applyTunnelModeTransform, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().applyTunnelModeTransform(tunnelResourceId, direction, transformResourceId, callingPackage);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      @Override public void removeTransportModeTransforms(android.os.ParcelFileDescriptor socket) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((socket!=null)) {
            _data.writeInt(1);
            socket.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeTransportModeTransforms, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeTransportModeTransforms(socket);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.net.IIpSecService sDefaultImpl;
    }
    static final int TRANSACTION_allocateSecurityParameterIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_releaseSecurityParameterIndex = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_openUdpEncapsulationSocket = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_closeUdpEncapsulationSocket = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_createTunnelInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_addAddressToTunnelInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeAddressFromTunnelInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_deleteTunnelInterface = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_createTransform = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_deleteTransform = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_applyTransportModeTransform = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_applyTunnelModeTransform = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_removeTransportModeTransforms = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    public static boolean setDefaultImpl(android.net.IIpSecService impl) {
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
    public static android.net.IIpSecService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public android.net.IpSecSpiResponse allocateSecurityParameterIndex(java.lang.String destinationAddress, int requestedSpi, android.os.IBinder binder) throws android.os.RemoteException;
  public void releaseSecurityParameterIndex(int resourceId) throws android.os.RemoteException;
  public android.net.IpSecUdpEncapResponse openUdpEncapsulationSocket(int port, android.os.IBinder binder) throws android.os.RemoteException;
  public void closeUdpEncapsulationSocket(int resourceId) throws android.os.RemoteException;
  public android.net.IpSecTunnelInterfaceResponse createTunnelInterface(java.lang.String localAddr, java.lang.String remoteAddr, android.net.Network underlyingNetwork, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException;
  public void addAddressToTunnelInterface(int tunnelResourceId, android.net.LinkAddress localAddr, java.lang.String callingPackage) throws android.os.RemoteException;
  public void removeAddressFromTunnelInterface(int tunnelResourceId, android.net.LinkAddress localAddr, java.lang.String callingPackage) throws android.os.RemoteException;
  public void deleteTunnelInterface(int resourceId, java.lang.String callingPackage) throws android.os.RemoteException;
  public android.net.IpSecTransformResponse createTransform(android.net.IpSecConfig c, android.os.IBinder binder, java.lang.String callingPackage) throws android.os.RemoteException;
  public void deleteTransform(int transformId) throws android.os.RemoteException;
  public void applyTransportModeTransform(android.os.ParcelFileDescriptor socket, int direction, int transformId) throws android.os.RemoteException;
  public void applyTunnelModeTransform(int tunnelResourceId, int direction, int transformResourceId, java.lang.String callingPackage) throws android.os.RemoteException;
  public void removeTransportModeTransforms(android.os.ParcelFileDescriptor socket) throws android.os.RemoteException;
}
