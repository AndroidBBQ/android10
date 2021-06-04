/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.vms;
/**
 * Exposes publisher services to VMS clients.
 *
 * @hide
 */
public interface IVmsPublisherService extends android.os.IInterface
{
  /** Default implementation for IVmsPublisherService. */
  public static class Default implements android.car.vms.IVmsPublisherService
  {
    /**
         * Client call to publish a message.
         */
    @Override public void publish(android.os.IBinder token, android.car.vms.VmsLayer layer, int publisherId, byte[] message) throws android.os.RemoteException
    {
    }
    /**
         * Returns the list of VmsLayers that has any clients subscribed to it.
         */
    @Override public android.car.vms.VmsSubscriptionState getSubscriptions() throws android.os.RemoteException
    {
      return null;
    }
    /**
         * Sets which layers the publisher can publish under which dependencties.
         */
    @Override public void setLayersOffering(android.os.IBinder token, android.car.vms.VmsLayersOffering offering) throws android.os.RemoteException
    {
    }
    /**
         * The first time a publisher calls this API it will store the publisher info and assigns the
         * publisher an ID. Between reboots, subsequent calls with the same publisher info will
         * return the same ID so that a restarting process can obtain the same ID as it had before.
         */
    @Override public int getPublisherId(byte[] publisherInfo) throws android.os.RemoteException
    {
      return 0;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.vms.IVmsPublisherService
  {
    private static final java.lang.String DESCRIPTOR = "android.car.vms.IVmsPublisherService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.vms.IVmsPublisherService interface,
     * generating a proxy if needed.
     */
    public static android.car.vms.IVmsPublisherService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.vms.IVmsPublisherService))) {
        return ((android.car.vms.IVmsPublisherService)iin);
      }
      return new android.car.vms.IVmsPublisherService.Stub.Proxy(obj);
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
        case TRANSACTION_publish:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.car.vms.VmsLayer _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.vms.VmsLayer.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          byte[] _arg3;
          _arg3 = data.createByteArray();
          this.publish(_arg0, _arg1, _arg2, _arg3);
          return true;
        }
        case TRANSACTION_getSubscriptions:
        {
          data.enforceInterface(descriptor);
          android.car.vms.VmsSubscriptionState _result = this.getSubscriptions();
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
        case TRANSACTION_setLayersOffering:
        {
          data.enforceInterface(descriptor);
          android.os.IBinder _arg0;
          _arg0 = data.readStrongBinder();
          android.car.vms.VmsLayersOffering _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.vms.VmsLayersOffering.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.setLayersOffering(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_getPublisherId:
        {
          data.enforceInterface(descriptor);
          byte[] _arg0;
          _arg0 = data.createByteArray();
          int _result = this.getPublisherId(_arg0);
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
    private static class Proxy implements android.car.vms.IVmsPublisherService
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
           * Client call to publish a message.
           */
      @Override public void publish(android.os.IBinder token, android.car.vms.VmsLayer layer, int publisherId, byte[] message) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((layer!=null)) {
            _data.writeInt(1);
            layer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(publisherId);
          _data.writeByteArray(message);
          boolean _status = mRemote.transact(Stub.TRANSACTION_publish, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().publish(token, layer, publisherId, message);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * Returns the list of VmsLayers that has any clients subscribed to it.
           */
      @Override public android.car.vms.VmsSubscriptionState getSubscriptions() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.vms.VmsSubscriptionState _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getSubscriptions, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getSubscriptions();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.vms.VmsSubscriptionState.CREATOR.createFromParcel(_reply);
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
      /**
           * Sets which layers the publisher can publish under which dependencties.
           */
      @Override public void setLayersOffering(android.os.IBinder token, android.car.vms.VmsLayersOffering offering) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder(token);
          if ((offering!=null)) {
            _data.writeInt(1);
            offering.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_setLayersOffering, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().setLayersOffering(token, offering);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /**
           * The first time a publisher calls this API it will store the publisher info and assigns the
           * publisher an ID. Between reboots, subsequent calls with the same publisher info will
           * return the same ID so that a restarting process can obtain the same ID as it had before.
           */
      @Override public int getPublisherId(byte[] publisherInfo) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        int _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeByteArray(publisherInfo);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPublisherId, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPublisherId(publisherInfo);
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
      public static android.car.vms.IVmsPublisherService sDefaultImpl;
    }
    static final int TRANSACTION_publish = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_getSubscriptions = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_setLayersOffering = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_getPublisherId = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.car.vms.IVmsPublisherService impl) {
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
    public static android.car.vms.IVmsPublisherService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Client call to publish a message.
       */
  public void publish(android.os.IBinder token, android.car.vms.VmsLayer layer, int publisherId, byte[] message) throws android.os.RemoteException;
  /**
       * Returns the list of VmsLayers that has any clients subscribed to it.
       */
  public android.car.vms.VmsSubscriptionState getSubscriptions() throws android.os.RemoteException;
  /**
       * Sets which layers the publisher can publish under which dependencties.
       */
  public void setLayersOffering(android.os.IBinder token, android.car.vms.VmsLayersOffering offering) throws android.os.RemoteException;
  /**
       * The first time a publisher calls this API it will store the publisher info and assigns the
       * publisher an ID. Between reboots, subsequent calls with the same publisher info will
       * return the same ID so that a restarting process can obtain the same ID as it had before.
       */
  public int getPublisherId(byte[] publisherInfo) throws android.os.RemoteException;
}
