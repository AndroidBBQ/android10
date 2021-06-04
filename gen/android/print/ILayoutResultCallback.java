/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Callback for observing the result of android.print.PrintAdapter#onLayout.
 *
 * @hide
 */
public interface ILayoutResultCallback extends android.os.IInterface
{
  /** Default implementation for ILayoutResultCallback. */
  public static class Default implements android.print.ILayoutResultCallback
  {
    @Override public void onLayoutStarted(android.os.ICancellationSignal cancellation, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onLayoutFinished(android.print.PrintDocumentInfo info, boolean changed, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onLayoutFailed(java.lang.CharSequence error, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onLayoutCanceled(int sequence) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.ILayoutResultCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.print.ILayoutResultCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.ILayoutResultCallback interface,
     * generating a proxy if needed.
     */
    public static android.print.ILayoutResultCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.ILayoutResultCallback))) {
        return ((android.print.ILayoutResultCallback)iin);
      }
      return new android.print.ILayoutResultCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onLayoutStarted:
        {
          data.enforceInterface(descriptor);
          android.os.ICancellationSignal _arg0;
          _arg0 = android.os.ICancellationSignal.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.onLayoutStarted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onLayoutFinished:
        {
          data.enforceInterface(descriptor);
          android.print.PrintDocumentInfo _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.print.PrintDocumentInfo.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          boolean _arg1;
          _arg1 = (0!=data.readInt());
          int _arg2;
          _arg2 = data.readInt();
          this.onLayoutFinished(_arg0, _arg1, _arg2);
          return true;
        }
        case TRANSACTION_onLayoutFailed:
        {
          data.enforceInterface(descriptor);
          java.lang.CharSequence _arg0;
          if (0!=data.readInt()) {
            _arg0 = android.text.TextUtils.CHAR_SEQUENCE_CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          int _arg1;
          _arg1 = data.readInt();
          this.onLayoutFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onLayoutCanceled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onLayoutCanceled(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.ILayoutResultCallback
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
      @Override public void onLayoutStarted(android.os.ICancellationSignal cancellation, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cancellation!=null))?(cancellation.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLayoutStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLayoutStarted(cancellation, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLayoutFinished(android.print.PrintDocumentInfo info, boolean changed, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((info!=null)) {
            _data.writeInt(1);
            info.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(((changed)?(1):(0)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLayoutFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLayoutFinished(info, changed, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLayoutFailed(java.lang.CharSequence error, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if (error!=null) {
            _data.writeInt(1);
            android.text.TextUtils.writeToParcel(error, _data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLayoutFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLayoutFailed(error, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onLayoutCanceled(int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onLayoutCanceled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onLayoutCanceled(sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.print.ILayoutResultCallback sDefaultImpl;
    }
    static final int TRANSACTION_onLayoutStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onLayoutFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onLayoutFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onLayoutCanceled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.print.ILayoutResultCallback impl) {
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
    public static android.print.ILayoutResultCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onLayoutStarted(android.os.ICancellationSignal cancellation, int sequence) throws android.os.RemoteException;
  public void onLayoutFinished(android.print.PrintDocumentInfo info, boolean changed, int sequence) throws android.os.RemoteException;
  public void onLayoutFailed(java.lang.CharSequence error, int sequence) throws android.os.RemoteException;
  public void onLayoutCanceled(int sequence) throws android.os.RemoteException;
}
