/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.print;
/**
 * Callback for observing the result of android.print.DocuemntAdapter#onWrite.
 *
 * @hide
 */
public interface IWriteResultCallback extends android.os.IInterface
{
  /** Default implementation for IWriteResultCallback. */
  public static class Default implements android.print.IWriteResultCallback
  {
    @Override public void onWriteStarted(android.os.ICancellationSignal cancellation, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onWriteFinished(android.print.PageRange[] pages, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onWriteFailed(java.lang.CharSequence error, int sequence) throws android.os.RemoteException
    {
    }
    @Override public void onWriteCanceled(int sequence) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.print.IWriteResultCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.print.IWriteResultCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.print.IWriteResultCallback interface,
     * generating a proxy if needed.
     */
    public static android.print.IWriteResultCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.print.IWriteResultCallback))) {
        return ((android.print.IWriteResultCallback)iin);
      }
      return new android.print.IWriteResultCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onWriteStarted:
        {
          data.enforceInterface(descriptor);
          android.os.ICancellationSignal _arg0;
          _arg0 = android.os.ICancellationSignal.Stub.asInterface(data.readStrongBinder());
          int _arg1;
          _arg1 = data.readInt();
          this.onWriteStarted(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onWriteFinished:
        {
          data.enforceInterface(descriptor);
          android.print.PageRange[] _arg0;
          _arg0 = data.createTypedArray(android.print.PageRange.CREATOR);
          int _arg1;
          _arg1 = data.readInt();
          this.onWriteFinished(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onWriteFailed:
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
          this.onWriteFailed(_arg0, _arg1);
          return true;
        }
        case TRANSACTION_onWriteCanceled:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          this.onWriteCanceled(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.print.IWriteResultCallback
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
      @Override public void onWriteStarted(android.os.ICancellationSignal cancellation, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeStrongBinder((((cancellation!=null))?(cancellation.asBinder()):(null)));
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWriteStarted, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWriteStarted(cancellation, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onWriteFinished(android.print.PageRange[] pages, int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeTypedArray(pages, 0);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWriteFinished, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWriteFinished(pages, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onWriteFailed(java.lang.CharSequence error, int sequence) throws android.os.RemoteException
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
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWriteFailed, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWriteFailed(error, sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      @Override public void onWriteCanceled(int sequence) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(sequence);
          boolean _status = mRemote.transact(Stub.TRANSACTION_onWriteCanceled, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onWriteCanceled(sequence);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.print.IWriteResultCallback sDefaultImpl;
    }
    static final int TRANSACTION_onWriteStarted = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_onWriteFinished = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    static final int TRANSACTION_onWriteFailed = (android.os.IBinder.FIRST_CALL_TRANSACTION + 2);
    static final int TRANSACTION_onWriteCanceled = (android.os.IBinder.FIRST_CALL_TRANSACTION + 3);
    public static boolean setDefaultImpl(android.print.IWriteResultCallback impl) {
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
    public static android.print.IWriteResultCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void onWriteStarted(android.os.ICancellationSignal cancellation, int sequence) throws android.os.RemoteException;
  public void onWriteFinished(android.print.PageRange[] pages, int sequence) throws android.os.RemoteException;
  public void onWriteFailed(java.lang.CharSequence error, int sequence) throws android.os.RemoteException;
  public void onWriteCanceled(int sequence) throws android.os.RemoteException;
}
