/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package com.android.ims.internal;
/**
 * Provides the Ut interface interworking to get/set the supplementary service configuration.
 *
 * {@hide}
 */
public interface IImsUt extends android.os.IInterface
{
  /** Default implementation for IImsUt. */
  public static class Default implements com.android.ims.internal.IImsUt
  {
    /**
         * Closes the object. This object is not usable after being closed.
         */
    @Override public void close() throws android.os.RemoteException
    {
    }
    /**
         * Retrieves the configuration of the call barring.
         */
    @Override public int queryCallBarring(int cbType) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Retrieves the configuration of the call forward.
         */
    @Override public int queryCallForward(int condition, java.lang.String number) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Retrieves the configuration of the call waiting.
         */
    @Override public int queryCallWaiting() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Retrieves the default CLIR setting.
         */
    @Override public int queryCLIR() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Retrieves the CLIP call setting.
         */
    @Override public int queryCLIP() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Retrieves the COLR call setting.
         */
    @Override public int queryCOLR() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Retrieves the COLP call setting.
         */
    @Override public int queryCOLP() throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates or retrieves the supplementary service configuration.
         */
    @Override public int transact(android.os.Bundle ssInfo) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the call barring.
         */
    @Override public int updateCallBarring(int cbType, int action, java.lang.String[] barrList) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the call forward.
         */
    @Override public int updateCallForward(int action, int condition, java.lang.String number, int serviceClass, int timeSeconds) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the call waiting.
         */
    @Override public int updateCallWaiting(boolean enable, int serviceClass) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the CLIR supplementary service.
         */
    @Override public int updateCLIR(int clirMode) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the CLIP supplementary service.
         */
    @Override public int updateCLIP(boolean enable) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the COLR supplementary service.
         */
    @Override public int updateCOLR(int presentation) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the COLP supplementary service.
         */
    @Override public int updateCOLP(boolean enable) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Sets the listener.
         */
    @Override public void setListener(com.android.ims.internal.IImsUtListener listener) throws android.os.RemoteException
    {
    }
    /**
         * Retrieves the configuration of the call barring for specified service class.
         */
    @Override public int queryCallBarringForServiceClass(int cbType, int serviceClass) throws android.os.RemoteException
    {
      return 0;
    }
    /**
         * Updates the configuration of the call barring for specified service class.
         */
    @Override public int updateCallBarringForServiceClass(int cbType, int action, java.lang.String[] barrList, int serviceClass) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements com.android.ims.internal.IImsUt
  {
    private static final java.lang.String DESCRIPTOR = "com.android.ims.internal.IImsUt";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an com.android.ims.internal.IImsUt interface,
     * generating a proxy if needed.
     */
    public static com.android.ims.internal.IImsUt asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof com.android.ims.internal.IImsUt))) {
        return ((com.android.ims.internal.IImsUt)iin);
      }
      return new com.android.ims.internal.IImsUt.Stub.Proxy(obj);
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
        case TRANSACTION_close:
        {
          data.enforceInterface(descriptor);
          this.close();
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_queryCallBarring:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.queryCallBarring(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_queryCallForward:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          java.lang.String _arg1;
          _arg1 = data.readString();
          int _result = this.queryCallForward(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_queryCallWaiting:
        {
          data.enforceInterface(descriptor);
          int _result = this.queryCallWaiting();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_queryCLIR:
        {
          data.enforceInterface(descriptor);
          int _result = this.queryCLIR();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_queryCLIP:
        {
          data.enforceInterface(descriptor);
          int _result = this.queryCLIP();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_queryCOLR:
        {
          data.enforceInterface(descriptor);
          int _result = this.queryCOLR();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_queryCOLP:
        {
          data.enforceInterface(descriptor);
          int _result = this.queryCOLP();
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_transact:
        {
          data.enforceInterface(descriptor);
          android.os.Bundle _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _result = this.transact(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCallBarring:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          int _result = this.updateCallBarring(_arg0, _arg1, _arg2);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCallForward:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String _arg2;
          _arg2 = data.readString();
          int _arg3;
          _arg3 = data.readInt();
          int _arg4;
          _arg4 = data.readInt();
          int _result = this.updateCallForward(_arg0, _arg1, _arg2, _arg3, _arg4);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCallWaiting:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.updateCallWaiting(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCLIR:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.updateCLIR(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCLIP:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _result = this.updateCLIP(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCOLR:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _result = this.updateCOLR(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCOLP:
        {
          data.enforceInterface(descriptor);
          boolean _arg0;
          _arg0 = (0!=data.readInt());
          int _result = this.updateCOLP(_arg0);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_setListener:
        {
          data.enforceInterface(descriptor);
          com.android.ims.internal.IImsUtListener _arg0;
          _arg0 = com.android.ims.internal.IImsUtListener.Stub.asInterface(data.readStrongBinder());
          this.setListener(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_queryCallBarringForServiceClass:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          int _result = this.queryCallBarringForServiceClass(_arg0, _arg1);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        case TRANSACTION_updateCallBarringForServiceClass:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          int _arg1;
          _arg1 = data.readInt();
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          int _arg3;
          _arg3 = data.readInt();
          int _result = this.updateCallBarringForServiceClass(_arg0, _arg1, _arg2, _arg3);
          reply.writeNoException();
          reply.writeInt(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements com.android.ims.internal.IImsUt
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
      /**
           * Closes the object. This object is not usable after being closed.
           */
      @Override public void close() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_close, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().close();
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Retrieves the configuration of the call barring.
           */
      @Override public int queryCallBarring(int cbType) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cbType);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCallBarring, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCallBarring(cbType);
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
      /**
           * Retrieves the configuration of the call forward.
           */
      @Override public int queryCallForward(int condition, java.lang.String number) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(condition);
          _data.writeString(number);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCallForward, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCallForward(condition, number);
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
      /**
           * Retrieves the configuration of the call waiting.
           */
      @Override public int queryCallWaiting() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCallWaiting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCallWaiting();
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
      /**
           * Retrieves the default CLIR setting.
           */
      @Override public int queryCLIR() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCLIR, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCLIR();
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
      /**
           * Retrieves the CLIP call setting.
           */
      @Override public int queryCLIP() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCLIP, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCLIP();
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
      /**
           * Retrieves the COLR call setting.
           */
      @Override public int queryCOLR() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCOLR, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCOLR();
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
      /**
           * Retrieves the COLP call setting.
           */
      @Override public int queryCOLP() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCOLP, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCOLP();
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
      /**
           * Updates or retrieves the supplementary service configuration.
           */
      @Override public int transact(android.os.Bundle ssInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((ssInfo!=null)) {
            _data.writeInt(1);
            ssInfo.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_transact, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().transact(ssInfo);
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
      /**
           * Updates the configuration of the call barring.
           */
      @Override public int updateCallBarring(int cbType, int action, java.lang.String[] barrList) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cbType);
          _data.writeInt(action);
          _data.writeStringArray(barrList);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCallBarring, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCallBarring(cbType, action, barrList);
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
      /**
           * Updates the configuration of the call forward.
           */
      @Override public int updateCallForward(int action, int condition, java.lang.String number, int serviceClass, int timeSeconds) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(action);
          _data.writeInt(condition);
          _data.writeString(number);
          _data.writeInt(serviceClass);
          _data.writeInt(timeSeconds);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCallForward, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCallForward(action, condition, number, serviceClass, timeSeconds);
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
      /**
           * Updates the configuration of the call waiting.
           */
      @Override public int updateCallWaiting(boolean enable, int serviceClass) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          _data.writeInt(serviceClass);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCallWaiting, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCallWaiting(enable, serviceClass);
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
      /**
           * Updates the configuration of the CLIR supplementary service.
           */
      @Override public int updateCLIR(int clirMode) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(clirMode);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCLIR, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCLIR(clirMode);
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
      /**
           * Updates the configuration of the CLIP supplementary service.
           */
      @Override public int updateCLIP(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCLIP, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCLIP(enable);
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
      /**
           * Updates the configuration of the COLR supplementary service.
           */
      @Override public int updateCOLR(int presentation) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(presentation);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCOLR, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCOLR(presentation);
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
      /**
           * Updates the configuration of the COLP supplementary service.
           */
      @Override public int updateCOLP(boolean enable) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(((enable)?(1):(0)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCOLP, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCOLP(enable);
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
      /**
           * Sets the listener.
           */
      @Override public void setListener(com.android.ims.internal.IImsUtListener listener) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((listener!=null))?(listener.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_setListener, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setListener(listener);
            return;
          }
          _reply.readException();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
      }
      /**
           * Retrieves the configuration of the call barring for specified service class.
           */
      @Override public int queryCallBarringForServiceClass(int cbType, int serviceClass) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cbType);
          _data.writeInt(serviceClass);
          boolean _status = mRemote.transact(Stub.TRANSACTION_queryCallBarringForServiceClass, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().queryCallBarringForServiceClass(cbType, serviceClass);
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
      /**
           * Updates the configuration of the call barring for specified service class.
           */
      @Override public int updateCallBarringForServiceClass(int cbType, int action, java.lang.String[] barrList, int serviceClass) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(cbType);
          _data.writeInt(action);
          _data.writeStringArray(barrList);
          _data.writeInt(serviceClass);
          boolean _status = mRemote.transact(Stub.TRANSACTION_updateCallBarringForServiceClass, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().updateCallBarringForServiceClass(cbType, action, barrList, serviceClass);
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
      public static com.android.ims.internal.IImsUt sDefaultImpl;
    }
    static final int TRANSACTION_close = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_queryCallBarring = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_queryCallForward = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_queryCallWaiting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_queryCLIR = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_queryCLIP = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_queryCOLR = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_queryCOLP = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_transact = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_updateCallBarring = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    static final int TRANSACTION_updateCallForward = (android.os.IBinder.FIRST_CALL_TRANSACTION + 10);
    static final int TRANSACTION_updateCallWaiting = (android.os.IBinder.FIRST_CALL_TRANSACTION + 11);
    static final int TRANSACTION_updateCLIR = (android.os.IBinder.FIRST_CALL_TRANSACTION + 12);
    static final int TRANSACTION_updateCLIP = (android.os.IBinder.FIRST_CALL_TRANSACTION + 13);
    static final int TRANSACTION_updateCOLR = (android.os.IBinder.FIRST_CALL_TRANSACTION + 14);
    static final int TRANSACTION_updateCOLP = (android.os.IBinder.FIRST_CALL_TRANSACTION + 15);
    static final int TRANSACTION_setListener = (android.os.IBinder.FIRST_CALL_TRANSACTION + 16);
    static final int TRANSACTION_queryCallBarringForServiceClass = (android.os.IBinder.FIRST_CALL_TRANSACTION + 17);
    static final int TRANSACTION_updateCallBarringForServiceClass = (android.os.IBinder.FIRST_CALL_TRANSACTION + 18);
    public static boolean setDefaultImpl(com.android.ims.internal.IImsUt impl) {
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
    public static com.android.ims.internal.IImsUt getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Closes the object. This object is not usable after being closed.
       */
  public void close() throws android.os.RemoteException;
  /**
       * Retrieves the configuration of the call barring.
       */
  public int queryCallBarring(int cbType) throws android.os.RemoteException;
  /**
       * Retrieves the configuration of the call forward.
       */
  public int queryCallForward(int condition, java.lang.String number) throws android.os.RemoteException;
  /**
       * Retrieves the configuration of the call waiting.
       */
  public int queryCallWaiting() throws android.os.RemoteException;
  /**
       * Retrieves the default CLIR setting.
       */
  public int queryCLIR() throws android.os.RemoteException;
  /**
       * Retrieves the CLIP call setting.
       */
  public int queryCLIP() throws android.os.RemoteException;
  /**
       * Retrieves the COLR call setting.
       */
  public int queryCOLR() throws android.os.RemoteException;
  /**
       * Retrieves the COLP call setting.
       */
  public int queryCOLP() throws android.os.RemoteException;
  /**
       * Updates or retrieves the supplementary service configuration.
       */
  public int transact(android.os.Bundle ssInfo) throws android.os.RemoteException;
  /**
       * Updates the configuration of the call barring.
       */
  public int updateCallBarring(int cbType, int action, java.lang.String[] barrList) throws android.os.RemoteException;
  /**
       * Updates the configuration of the call forward.
       */
  public int updateCallForward(int action, int condition, java.lang.String number, int serviceClass, int timeSeconds) throws android.os.RemoteException;
  /**
       * Updates the configuration of the call waiting.
       */
  public int updateCallWaiting(boolean enable, int serviceClass) throws android.os.RemoteException;
  /**
       * Updates the configuration of the CLIR supplementary service.
       */
  public int updateCLIR(int clirMode) throws android.os.RemoteException;
  /**
       * Updates the configuration of the CLIP supplementary service.
       */
  public int updateCLIP(boolean enable) throws android.os.RemoteException;
  /**
       * Updates the configuration of the COLR supplementary service.
       */
  public int updateCOLR(int presentation) throws android.os.RemoteException;
  /**
       * Updates the configuration of the COLP supplementary service.
       */
  public int updateCOLP(boolean enable) throws android.os.RemoteException;
  /**
       * Sets the listener.
       */
  public void setListener(com.android.ims.internal.IImsUtListener listener) throws android.os.RemoteException;
  /**
       * Retrieves the configuration of the call barring for specified service class.
       */
  public int queryCallBarringForServiceClass(int cbType, int serviceClass) throws android.os.RemoteException;
  /**
       * Updates the configuration of the call barring for specified service class.
       */
  public int updateCallBarringForServiceClass(int cbType, int action, java.lang.String[] barrList, int serviceClass) throws android.os.RemoteException;
}
