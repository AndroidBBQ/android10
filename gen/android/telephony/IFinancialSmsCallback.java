/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.telephony;
/** Interface for returning back the financial sms messages asynchrously.
 *  @hide
 */
public interface IFinancialSmsCallback extends android.os.IInterface
{
  /** Default implementation for IFinancialSmsCallback. */
  public static class Default implements android.telephony.IFinancialSmsCallback
  {
    /**
         * Return sms messages back to calling financial app.
         *
         * @param messages the sms messages returned for cinancial app.
         */
    @Override public void onGetSmsMessagesForFinancialApp(android.database.CursorWindow messages) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.telephony.IFinancialSmsCallback
  {
    private static final java.lang.String DESCRIPTOR = "android.telephony.IFinancialSmsCallback";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.telephony.IFinancialSmsCallback interface,
     * generating a proxy if needed.
     */
    public static android.telephony.IFinancialSmsCallback asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.telephony.IFinancialSmsCallback))) {
        return ((android.telephony.IFinancialSmsCallback)iin);
      }
      return new android.telephony.IFinancialSmsCallback.Stub.Proxy(obj);
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
        case TRANSACTION_onGetSmsMessagesForFinancialApp:
        {
          data.enforceInterface(descriptor);
          android.database.CursorWindow _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.database.CursorWindow.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.onGetSmsMessagesForFinancialApp(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.telephony.IFinancialSmsCallback
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
           * Return sms messages back to calling financial app.
           *
           * @param messages the sms messages returned for cinancial app.
           */
      @Override public void onGetSmsMessagesForFinancialApp(android.database.CursorWindow messages) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((messages!=null)) {
            _data.writeInt(1);
            messages.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onGetSmsMessagesForFinancialApp, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onGetSmsMessagesForFinancialApp(messages);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.telephony.IFinancialSmsCallback sDefaultImpl;
    }
    static final int TRANSACTION_onGetSmsMessagesForFinancialApp = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.telephony.IFinancialSmsCallback impl) {
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
    public static android.telephony.IFinancialSmsCallback getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Return sms messages back to calling financial app.
       *
       * @param messages the sms messages returned for cinancial app.
       */
  public void onGetSmsMessagesForFinancialApp(android.database.CursorWindow messages) throws android.os.RemoteException;
}
