/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.hardware.location;
/**
 * @hide
 */
public interface IContextHubService extends android.os.IInterface
{
  /** Default implementation for IContextHubService. */
  public static class Default implements android.hardware.location.IContextHubService
  {
    // Registers a callback to receive messages

    @Override public int registerCallback(android.hardware.location.IContextHubCallback callback) throws android.os.RemoteException
    {
      return 0;
    }
    // Gets a list of available context hub handles

    @Override public int[] getContextHubHandles() throws android.os.RemoteException
    {
      return null;
    }
    // Gets the properties of a hub

    @Override public android.hardware.location.ContextHubInfo getContextHubInfo(int contextHubHandle) throws android.os.RemoteException
    {
      return null;
    }
    // Loads a nanoapp at the specified hub (old API)

    @Override public int loadNanoApp(int contextHubHandle, android.hardware.location.NanoApp nanoApp) throws android.os.RemoteException
    {
      return 0;
    }
    // Unloads a nanoapp given its instance ID (old API)

    @Override public int unloadNanoApp(int nanoAppHandle) throws android.os.RemoteException
    {
      return 0;
    }
    // Gets the NanoAppInstanceInfo of a nanoapp give its instance ID

    @Override public android.hardware.location.NanoAppInstanceInfo getNanoAppInstanceInfo(int nanoAppHandle) throws android.os.RemoteException
    {
      return null;
    }
    // Finds all nanoApp instances matching some filter

    @Override public int[] findNanoAppOnHub(int contextHubHandle, android.hardware.location.NanoAppFilter filter) throws android.os.RemoteException
    {
      return null;
    }
    // Sends a message to a nanoApp

    @Override public int sendMessage(int contextHubHandle, int nanoAppHandle, android.hardware.location.ContextHubMessage msg) throws android.os.RemoteException
    {
      return 0;
    }
    // Creates a client to send and receive messages

    @Override public android.hardware.location.IContextHubClient createClient(int contextHubId, android.hardware.location.IContextHubClientCallback client) throws android.os.RemoteException
    {
      return null;
    }
    // Creates a PendingIntent-based client to send and receive messages

    @Override public android.hardware.location.IContextHubClient createPendingIntentClient(int contextHubId, android.app.PendingIntent pendingIntent, long nanoAppId) throws android.os.RemoteException
    {
      return null;
    }
    // Returns a list of ContextHub objects of available hubs

    @Override public java.util.List<android.hardware.location.ContextHubInfo> getContextHubs() throws android.os.RemoteException
    {
      return null;
    }
    // Loads a nanoapp at the specified hub (new API)

    @Override public void loadNanoAppOnHub(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, android.hardware.location.NanoAppBinary nanoAppBinary) throws android.os.RemoteException
    {
    }
    // Unloads a nanoapp on a specified context hub (new API)

    @Override public void unloadNanoAppFromHub(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException
    {
    }
    // Enables a nanoapp at the specified hub

    @Override public void enableNanoApp(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException
    {
    }
    // Disables a nanoapp at the specified hub

    @Override public void disableNanoApp(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException
    {
    }
    // Queries for a list of nanoapps

    @Override public void queryNanoApps(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.hardware.location.IContextHubService
  {
    private static final java.lang.String DESCRIPTOR = "android.hardware.location.IContextHubService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.hardware.location.IContextHubService interface,
     * generating a proxy if needed.
     */
    public static android.hardware.location.IContextHubService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.hardware.location.IContextHubService))) {
        return ((android.hardware.location.IContextHubService)iin);
      }
      return new android.hardware.location.IContextHubService.Stub.Proxy(obj);
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
        case TRANSACTION_registerCallback:
        {
          data.enforceInterface(descriptor);
          android.hardware.location.IContextHubCallback _arg0;
          _arg0 = android.hardware.location.IContextHubCallback.Stub.asInterface(data.readStrongBinder());
          int _result = this.registerCallback(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getContextHubHandles:
        {
          data.enforceInterface(descriptor);
          int[] _result = this.getContextHubHandles();
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_getContextHubInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.ContextHubInfo _result = this.getContextHubInfo(_arg0);
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
        case TRANSACTION_loadNanoApp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.NanoApp _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.location.NanoApp.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _result = this.loadNanoApp(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_unloadNanoApp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.unloadNanoApp(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_getNanoAppInstanceInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.NanoAppInstanceInfo _result = this.getNanoAppInstanceInfo(_arg0);
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
        case TRANSACTION_findNanoAppOnHub:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.NanoAppFilter _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.hardware.location.NanoAppFilter.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int[] _result = this.findNanoAppOnHub(_arg0, _arg1);
          reply.writeNoException();
          reply.writeIntArray(_result);
          return true;
        }
        case TRANSACTION_sendMessage:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          android.hardware.location.ContextHubMessage _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.hardware.location.ContextHubMessage.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          int _result = this.sendMessage(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_createClient:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IContextHubClientCallback _arg1;
          _arg1 = android.hardware.location.IContextHubClientCallback.Stub.asInterface(data.readStrongBinder());
          android.hardware.location.IContextHubClient _result = this.createClient(_arg0, _arg1);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_createPendingIntentClient:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.app.PendingIntent _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.app.PendingIntent.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          long _arg2;
          _arg2 = data.readLong();
          android.hardware.location.IContextHubClient _result = this.createPendingIntentClient(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeStrongBinder((((_result!=null))?(_result.asBinder()):(null)));
          return true;
        }
        case TRANSACTION_getContextHubs:
        {
          data.enforceInterface(descriptor);
          java.util.List<android.hardware.location.ContextHubInfo> _result = this.getContextHubs();
          reply.writeNoException();
          reply.writeTypedList(_result);
          return true;
        }
        case TRANSACTION_loadNanoAppOnHub:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IContextHubTransactionCallback _arg1;
          _arg1 = android.hardware.location.IContextHubTransactionCallback.Stub.asInterface(data.readStrongBinder());
          android.hardware.location.NanoAppBinary _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.hardware.location.NanoAppBinary.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.loadNanoAppOnHub(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_unloadNanoAppFromHub:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IContextHubTransactionCallback _arg1;
          _arg1 = android.hardware.location.IContextHubTransactionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg2;
          _arg2 = data.readLong();
          this.unloadNanoAppFromHub(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_enableNanoApp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IContextHubTransactionCallback _arg1;
          _arg1 = android.hardware.location.IContextHubTransactionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg2;
          _arg2 = data.readLong();
          this.enableNanoApp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_disableNanoApp:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IContextHubTransactionCallback _arg1;
          _arg1 = android.hardware.location.IContextHubTransactionCallback.Stub.asInterface(data.readStrongBinder());
          long _arg2;
          _arg2 = data.readLong();
          this.disableNanoApp(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_queryNanoApps:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.hardware.location.IContextHubTransactionCallback _arg1;
          _arg1 = android.hardware.location.IContextHubTransactionCallback.Stub.asInterface(data.readStrongBinder());
          this.queryNanoApps(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.hardware.location.IContextHubService
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
      // Registers a callback to receive messages

      @Override public int registerCallback(android.hardware.location.IContextHubCallback callback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((callback!=null))?(callback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_registerCallback, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().registerCallback(callback);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Gets a list of available context hub handles

      @Override public int[] getContextHubHandles() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContextHubHandles, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContextHubHandles();
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Gets the properties of a hub

      @Override public android.hardware.location.ContextHubInfo getContextHubInfo(int contextHubHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.location.ContextHubInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContextHubInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContextHubInfo(contextHubHandle);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.location.ContextHubInfo.CREATOR.createFromParcel(_reply);
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
      // Loads a nanoapp at the specified hub (old API)

      @Override public int loadNanoApp(int contextHubHandle, android.hardware.location.NanoApp nanoApp) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubHandle);
          if ((nanoApp!=null)) {
            _data.writeInt(1);
            nanoApp.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadNanoApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().loadNanoApp(contextHubHandle, nanoApp);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Unloads a nanoapp given its instance ID (old API)

      @Override public int unloadNanoApp(int nanoAppHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(nanoAppHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unloadNanoApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().unloadNanoApp(nanoAppHandle);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Gets the NanoAppInstanceInfo of a nanoapp give its instance ID

      @Override public android.hardware.location.NanoAppInstanceInfo getNanoAppInstanceInfo(int nanoAppHandle) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.location.NanoAppInstanceInfo _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(nanoAppHandle);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getNanoAppInstanceInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getNanoAppInstanceInfo(nanoAppHandle);
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.hardware.location.NanoAppInstanceInfo.CREATOR.createFromParcel(_reply);
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
      // Finds all nanoApp instances matching some filter

      @Override public int[] findNanoAppOnHub(int contextHubHandle, android.hardware.location.NanoAppFilter filter) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubHandle);
          if ((filter!=null)) {
            _data.writeInt(1);
            filter.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_findNanoAppOnHub, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().findNanoAppOnHub(contextHubHandle, filter);
          }
          _reply.readException();
          _result = _reply.createIntArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Sends a message to a nanoApp

      @Override public int sendMessage(int contextHubHandle, int nanoAppHandle, android.hardware.location.ContextHubMessage msg) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubHandle);
          _data.writeInt(nanoAppHandle);
          if ((msg!=null)) {
            _data.writeInt(1);
            msg.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_sendMessage, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().sendMessage(contextHubHandle, nanoAppHandle, msg);
          }
          _reply.readException();
          _result = _reply.readInt();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Creates a client to send and receive messages

      @Override public android.hardware.location.IContextHubClient createClient(int contextHubId, android.hardware.location.IContextHubClientCallback client) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.location.IContextHubClient _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          _data.writeStrongBinder((((client!=null))?(client.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_createClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createClient(contextHubId, client);
          }
          _reply.readException();
          _result = android.hardware.location.IContextHubClient.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Creates a PendingIntent-based client to send and receive messages

      @Override public android.hardware.location.IContextHubClient createPendingIntentClient(int contextHubId, android.app.PendingIntent pendingIntent, long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.hardware.location.IContextHubClient _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          if ((pendingIntent!=null)) {
            _data.writeInt(1);
            pendingIntent.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_createPendingIntentClient, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().createPendingIntentClient(contextHubId, pendingIntent, nanoAppId);
          }
          _reply.readException();
          _result = android.hardware.location.IContextHubClient.Stub.asInterface(_reply.readStrongBinder());
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Returns a list of ContextHub objects of available hubs

      @Override public java.util.List<android.hardware.location.ContextHubInfo> getContextHubs() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        java.util.List<android.hardware.location.ContextHubInfo> _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getContextHubs, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getContextHubs();
          }
          _reply.readException();
          _result = _reply.createTypedArrayList(android.hardware.location.ContextHubInfo.CREATOR);
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      // Loads a nanoapp at the specified hub (new API)

      @Override public void loadNanoAppOnHub(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, android.hardware.location.NanoAppBinary nanoAppBinary) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          _data.writeStrongBinder((((transactionCallback!=null))?(transactionCallback.asBinder()):(null)));
          if ((nanoAppBinary!=null)) {
            _data.writeInt(1);
            nanoAppBinary.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_loadNanoAppOnHub, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().loadNanoAppOnHub(contextHubId, transactionCallback, nanoAppBinary);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Unloads a nanoapp on a specified context hub (new API)

      @Override public void unloadNanoAppFromHub(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          _data.writeStrongBinder((((transactionCallback!=null))?(transactionCallback.asBinder()):(null)));
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_unloadNanoAppFromHub, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().unloadNanoAppFromHub(contextHubId, transactionCallback, nanoAppId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Enables a nanoapp at the specified hub

      @Override public void enableNanoApp(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          _data.writeStrongBinder((((transactionCallback!=null))?(transactionCallback.asBinder()):(null)));
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_enableNanoApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().enableNanoApp(contextHubId, transactionCallback, nanoAppId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Disables a nanoapp at the specified hub

      @Override public void disableNanoApp(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          _data.writeStrongBinder((((transactionCallback!=null))?(transactionCallback.asBinder()):(null)));
          _data.writeLong(nanoAppId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_disableNanoApp, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().disableNanoApp(contextHubId, transactionCallback, nanoAppId);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      // Queries for a list of nanoapps

      @Override public void queryNanoApps(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(contextHubId);
          _data.writeStrongBinder((((transactionCallback!=null))?(transactionCallback.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryNanoApps, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().queryNanoApps(contextHubId, transactionCallback);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      public static android.hardware.location.IContextHubService sDefaultImpl;
    }
    static final int TRANSACTION_registerCallback = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getContextHubHandles = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_getContextHubInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_loadNanoApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_unloadNanoApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_getNanoAppInstanceInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_findNanoAppOnHub = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_sendMessage = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_createClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_createPendingIntentClient = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_getContextHubs = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_loadNanoAppOnHub = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_unloadNanoAppFromHub = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_enableNanoApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_disableNanoApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_queryNanoApps = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    public static boolean setDefaultImpl(android.hardware.location.IContextHubService impl) {
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
    public static android.hardware.location.IContextHubService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  // Registers a callback to receive messages

  public int registerCallback(android.hardware.location.IContextHubCallback callback) throws android.os.RemoteException;
  // Gets a list of available context hub handles

  public int[] getContextHubHandles() throws android.os.RemoteException;
  // Gets the properties of a hub

  public android.hardware.location.ContextHubInfo getContextHubInfo(int contextHubHandle) throws android.os.RemoteException;
  // Loads a nanoapp at the specified hub (old API)

  public int loadNanoApp(int contextHubHandle, android.hardware.location.NanoApp nanoApp) throws android.os.RemoteException;
  // Unloads a nanoapp given its instance ID (old API)

  public int unloadNanoApp(int nanoAppHandle) throws android.os.RemoteException;
  // Gets the NanoAppInstanceInfo of a nanoapp give its instance ID

  public android.hardware.location.NanoAppInstanceInfo getNanoAppInstanceInfo(int nanoAppHandle) throws android.os.RemoteException;
  // Finds all nanoApp instances matching some filter

  public int[] findNanoAppOnHub(int contextHubHandle, android.hardware.location.NanoAppFilter filter) throws android.os.RemoteException;
  // Sends a message to a nanoApp

  public int sendMessage(int contextHubHandle, int nanoAppHandle, android.hardware.location.ContextHubMessage msg) throws android.os.RemoteException;
  // Creates a client to send and receive messages

  public android.hardware.location.IContextHubClient createClient(int contextHubId, android.hardware.location.IContextHubClientCallback client) throws android.os.RemoteException;
  // Creates a PendingIntent-based client to send and receive messages

  public android.hardware.location.IContextHubClient createPendingIntentClient(int contextHubId, android.app.PendingIntent pendingIntent, long nanoAppId) throws android.os.RemoteException;
  // Returns a list of ContextHub objects of available hubs

  public java.util.List<android.hardware.location.ContextHubInfo> getContextHubs() throws android.os.RemoteException;
  // Loads a nanoapp at the specified hub (new API)

  public void loadNanoAppOnHub(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, android.hardware.location.NanoAppBinary nanoAppBinary) throws android.os.RemoteException;
  // Unloads a nanoapp on a specified context hub (new API)

  public void unloadNanoAppFromHub(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException;
  // Enables a nanoapp at the specified hub

  public void enableNanoApp(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException;
  // Disables a nanoapp at the specified hub

  public void disableNanoApp(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback, long nanoAppId) throws android.os.RemoteException;
  // Queries for a list of nanoapps

  public void queryNanoApps(int contextHubId, android.hardware.location.IContextHubTransactionCallback transactionCallback) throws android.os.RemoteException;
}
