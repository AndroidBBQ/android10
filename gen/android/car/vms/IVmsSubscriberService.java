/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.car.vms;
/**
 * @hide
 */
public interface IVmsSubscriberService extends android.os.IInterface
{
  /** Default implementation for IVmsSubscriberService. */
  public static class Default implements android.car.vms.IVmsSubscriberService
  {
    /**
         * Adds a subscriber to notifications only.
         * Should be called when a subscriber registers its callback, and before any subscription to a
         * layer is made.
         */
    @Override public void addVmsSubscriberToNotifications(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
    {
    }
    /**
         * Adds a subscriber to a VMS layer.
         */
    @Override public void addVmsSubscriber(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer) throws android.os.RemoteException
    {
    }
    /**
         * Adds a subscriber to all actively broadcasted layers.
         * Publishers will not be notified regarding this request so the state of the service will not
         * change.
         */
    @Override public void addVmsSubscriberPassive(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
    {
    }
    /**
         * Adds a subscriber to a VMS layer from a specific publisher.
         */
    @Override public void addVmsSubscriberToPublisher(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer, int publisherId) throws android.os.RemoteException
    {
    }
    /**
         * Removes a subscriber to notifications only.
         * Should be called when a subscriber unregisters its callback, and after all subscriptions to
         * layers are removed.
         */
    @Override public void removeVmsSubscriberToNotifications(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
    {
    }
    /**
         * Removes a subscriber to a VMS layer.
         */
    @Override public void removeVmsSubscriber(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer) throws android.os.RemoteException
    {
    }
    /**
         * Removes a subscriber to all actively broadcasted layers.
         * Publishers will not be notified regarding this request so the state of the service will not
         * change.
         */
    @Override public void removeVmsSubscriberPassive(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
    {
    }
    /**
         * Removes a subscriber to a VMS layer from a specific publisher.
         */
    @Override public void removeVmsSubscriberToPublisher(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer, int publisherId) throws android.os.RemoteException
    {
    }
    /**
         * Returns a list of available layers from the closure of the publishers offerings.
         */
    @Override public android.car.vms.VmsAvailableLayers getAvailableLayers() throws android.os.RemoteException
    {
      return null;
    }
    /**
         *  Returns a the publisher information for a publisher ID.
         */
    @Override public byte[] getPublisherInfo(int publisherId) throws android.os.RemoteException
    {
      return null;
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.car.vms.IVmsSubscriberService
  {
    private static final java.lang.String DESCRIPTOR = "android.car.vms.IVmsSubscriberService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.car.vms.IVmsSubscriberService interface,
     * generating a proxy if needed.
     */
    public static android.car.vms.IVmsSubscriberService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.car.vms.IVmsSubscriberService))) {
        return ((android.car.vms.IVmsSubscriberService)iin);
      }
      return new android.car.vms.IVmsSubscriberService.Stub.Proxy(obj);
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
        case TRANSACTION_addVmsSubscriberToNotifications:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          this.addVmsSubscriberToNotifications(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addVmsSubscriber:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          android.car.vms.VmsLayer _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.vms.VmsLayer.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.addVmsSubscriber(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addVmsSubscriberPassive:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          this.addVmsSubscriberPassive(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_addVmsSubscriberToPublisher:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          android.car.vms.VmsLayer _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.vms.VmsLayer.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.addVmsSubscriberToPublisher(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeVmsSubscriberToNotifications:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          this.removeVmsSubscriberToNotifications(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeVmsSubscriber:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          android.car.vms.VmsLayer _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.vms.VmsLayer.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          this.removeVmsSubscriber(_arg0, _arg1);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeVmsSubscriberPassive:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          this.removeVmsSubscriberPassive(_arg0);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_removeVmsSubscriberToPublisher:
        {
          data.enforceInterface(descriptor);
          android.car.vms.IVmsSubscriberClient _arg0;
          _arg0 = android.car.vms.IVmsSubscriberClient.Stub.asInterface(data.readStrongBinder());
          android.car.vms.VmsLayer _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.car.vms.VmsLayer.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          int _arg2;
          _arg2 = data.readInt();
          this.removeVmsSubscriberToPublisher(_arg0, _arg1, _arg2);
          reply.writeNoException();
          return true;
        }
        case TRANSACTION_getAvailableLayers:
        {
          data.enforceInterface(descriptor);
          android.car.vms.VmsAvailableLayers _result = this.getAvailableLayers();
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
        case TRANSACTION_getPublisherInfo:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          byte[] _result = this.getPublisherInfo(_arg0);
          reply.writeNoException();
          reply.writeByteArray(_result);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.car.vms.IVmsSubscriberService
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
           * Adds a subscriber to notifications only.
           * Should be called when a subscriber registers its callback, and before any subscription to a
           * layer is made.
           */
      @Override public void addVmsSubscriberToNotifications(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVmsSubscriberToNotifications, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addVmsSubscriberToNotifications(subscriber);
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
           * Adds a subscriber to a VMS layer.
           */
      @Override public void addVmsSubscriber(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          if ((layer!=null)) {
            _data.writeInt(1);
            layer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVmsSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addVmsSubscriber(subscriber, layer);
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
           * Adds a subscriber to all actively broadcasted layers.
           * Publishers will not be notified regarding this request so the state of the service will not
           * change.
           */
      @Override public void addVmsSubscriberPassive(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVmsSubscriberPassive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addVmsSubscriberPassive(subscriber);
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
           * Adds a subscriber to a VMS layer from a specific publisher.
           */
      @Override public void addVmsSubscriberToPublisher(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer, int publisherId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          if ((layer!=null)) {
            _data.writeInt(1);
            layer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(publisherId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_addVmsSubscriberToPublisher, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().addVmsSubscriberToPublisher(subscriber, layer, publisherId);
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
           * Removes a subscriber to notifications only.
           * Should be called when a subscriber unregisters its callback, and after all subscriptions to
           * layers are removed.
           */
      @Override public void removeVmsSubscriberToNotifications(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeVmsSubscriberToNotifications, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeVmsSubscriberToNotifications(subscriber);
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
           * Removes a subscriber to a VMS layer.
           */
      @Override public void removeVmsSubscriber(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          if ((layer!=null)) {
            _data.writeInt(1);
            layer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeVmsSubscriber, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeVmsSubscriber(subscriber, layer);
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
           * Removes a subscriber to all actively broadcasted layers.
           * Publishers will not be notified regarding this request so the state of the service will not
           * change.
           */
      @Override public void removeVmsSubscriberPassive(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeVmsSubscriberPassive, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeVmsSubscriberPassive(subscriber);
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
           * Removes a subscriber to a VMS layer from a specific publisher.
           */
      @Override public void removeVmsSubscriberToPublisher(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer, int publisherId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((subscriber!=null))?(subscriber.asBinder()):(null)));
          if ((layer!=null)) {
            _data.writeInt(1);
            layer.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(publisherId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_removeVmsSubscriberToPublisher, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().removeVmsSubscriberToPublisher(subscriber, layer, publisherId);
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
           * Returns a list of available layers from the closure of the publishers offerings.
           */
      @Override public android.car.vms.VmsAvailableLayers getAvailableLayers() throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        android.car.vms.VmsAvailableLayers _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getAvailableLayers, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getAvailableLayers();
          }
          _reply.readException();
          if ((0!=_reply.readInt())) {
            _result = android.car.vms.VmsAvailableLayers.CREATOR.createFromParcel(_reply);
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
           *  Returns a the publisher information for a publisher ID.
           */
      @Override public byte[] getPublisherInfo(int publisherId) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        android.os.Parcel _reply = android.os.Parcel.obtain();
        byte[] _result;
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(publisherId);
          boolean _status = mRemote.transact(Stub.TRANSACTION_getPublisherInfo, _data, _reply, 0);
          if (!_status && getDefaultImpl() != null) {
            return getDefaultImpl().getPublisherInfo(publisherId);
          }
          _reply.readException();
          _result = _reply.createByteArray();
        }
        finally {
          _reply.recycle();
          _data.recycle();
        }
        return _result;
      }
      public static android.car.vms.IVmsSubscriberService sDefaultImpl;
    }
    static final int TRANSACTION_addVmsSubscriberToNotifications = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_addVmsSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_addVmsSubscriberPassive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_addVmsSubscriberToPublisher = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    static final int TRANSACTION_removeVmsSubscriberToNotifications = (android.os.IBinder.FIRST_CALL_TRANSACTION + 4);
    static final int TRANSACTION_removeVmsSubscriber = (android.os.IBinder.FIRST_CALL_TRANSACTION + 5);
    static final int TRANSACTION_removeVmsSubscriberPassive = (android.os.IBinder.FIRST_CALL_TRANSACTION + 6);
    static final int TRANSACTION_removeVmsSubscriberToPublisher = (android.os.IBinder.FIRST_CALL_TRANSACTION + 7);
    static final int TRANSACTION_getAvailableLayers = (android.os.IBinder.FIRST_CALL_TRANSACTION + 8);
    static final int TRANSACTION_getPublisherInfo = (android.os.IBinder.FIRST_CALL_TRANSACTION + 9);
    public static boolean setDefaultImpl(android.car.vms.IVmsSubscriberService impl) {
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
    public static android.car.vms.IVmsSubscriberService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Adds a subscriber to notifications only.
       * Should be called when a subscriber registers its callback, and before any subscription to a
       * layer is made.
       */
  public void addVmsSubscriberToNotifications(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException;
  /**
       * Adds a subscriber to a VMS layer.
       */
  public void addVmsSubscriber(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer) throws android.os.RemoteException;
  /**
       * Adds a subscriber to all actively broadcasted layers.
       * Publishers will not be notified regarding this request so the state of the service will not
       * change.
       */
  public void addVmsSubscriberPassive(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException;
  /**
       * Adds a subscriber to a VMS layer from a specific publisher.
       */
  public void addVmsSubscriberToPublisher(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer, int publisherId) throws android.os.RemoteException;
  /**
       * Removes a subscriber to notifications only.
       * Should be called when a subscriber unregisters its callback, and after all subscriptions to
       * layers are removed.
       */
  public void removeVmsSubscriberToNotifications(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException;
  /**
       * Removes a subscriber to a VMS layer.
       */
  public void removeVmsSubscriber(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer) throws android.os.RemoteException;
  /**
       * Removes a subscriber to all actively broadcasted layers.
       * Publishers will not be notified regarding this request so the state of the service will not
       * change.
       */
  public void removeVmsSubscriberPassive(android.car.vms.IVmsSubscriberClient subscriber) throws android.os.RemoteException;
  /**
       * Removes a subscriber to a VMS layer from a specific publisher.
       */
  public void removeVmsSubscriberToPublisher(android.car.vms.IVmsSubscriberClient subscriber, android.car.vms.VmsLayer layer, int publisherId) throws android.os.RemoteException;
  /**
       * Returns a list of available layers from the closure of the publishers offerings.
       */
  public android.car.vms.VmsAvailableLayers getAvailableLayers() throws android.os.RemoteException;
  /**
       *  Returns a the publisher information for a publisher ID.
       */
  public byte[] getPublisherInfo(int publisherId) throws android.os.RemoteException;
}
