/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.service.autofill;
/**
 * Service used to calculate match scores for Autofill Field Classification.
 *
 * @hide
 */
public interface IAutofillFieldClassificationService extends android.os.IInterface
{
  /** Default implementation for IAutofillFieldClassificationService. */
  public static class Default implements android.service.autofill.IAutofillFieldClassificationService
  {
    @Override public void calculateScores(android.os.RemoteCallback callback, java.util.List<android.view.autofill.AutofillValue> actualValues, java.lang.String[] userDataValues, java.lang.String[] categoryIds, java.lang.String defaultAlgorithm, android.os.Bundle defaultArgs, java.util.Map algorithms, java.util.Map args) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.service.autofill.IAutofillFieldClassificationService
  {
    private static final java.lang.String DESCRIPTOR = "android.service.autofill.IAutofillFieldClassificationService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.service.autofill.IAutofillFieldClassificationService interface,
     * generating a proxy if needed.
     */
    public static android.service.autofill.IAutofillFieldClassificationService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.service.autofill.IAutofillFieldClassificationService))) {
        return ((android.service.autofill.IAutofillFieldClassificationService)iin);
      }
      return new android.service.autofill.IAutofillFieldClassificationService.Stub.Proxy(obj);
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
        case TRANSACTION_calculateScores:
        {
          data.enforceInterface(descriptor);
          android.os.RemoteCallback _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.os.RemoteCallback.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          java.util.List<android.view.autofill.AutofillValue> _arg1;
          _arg1 = data.createTypedArrayList(android.view.autofill.AutofillValue.CREATOR);
          java.lang.String[] _arg2;
          _arg2 = data.createStringArray();
          java.lang.String[] _arg3;
          _arg3 = data.createStringArray();
          java.lang.String _arg4;
          _arg4 = data.readString();
          android.os.Bundle _arg5;
          if ((0!=data.readInt())) {
            _arg5 = android.os.Bundle.CREATOR.createFromParcel(data);
          }
          else {
            _arg5 = null;
          }
          java.util.Map _arg6;
          java.lang.ClassLoader cl = (java.lang.ClassLoader)this.getClass().getClassLoader();
          _arg6 = data.readHashMap(cl);
          java.util.Map _arg7;
          _arg7 = data.readHashMap(cl);
          this.calculateScores(_arg0, _arg1, _arg2, _arg3, _arg4, _arg5, _arg6, _arg7);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.service.autofill.IAutofillFieldClassificationService
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
      @Override public void calculateScores(android.os.RemoteCallback callback, java.util.List<android.view.autofill.AutofillValue> actualValues, java.lang.String[] userDataValues, java.lang.String[] categoryIds, java.lang.String defaultAlgorithm, android.os.Bundle defaultArgs, java.util.Map algorithms, java.util.Map args) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((callback!=null)) {
            _data.writeInt(1);
            callback.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeTypedList(actualValues);
          _data.writeStringArray(userDataValues);
          _data.writeStringArray(categoryIds);
          _data.writeString(defaultAlgorithm);
          if ((defaultArgs!=null)) {
            _data.writeInt(1);
            defaultArgs.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          _data.writeMap(algorithms);
          _data.writeMap(args);
          boolean _status = mRemote.transact(Stub.TRANSACTION_calculateScores, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().calculateScores(callback, actualValues, userDataValues, categoryIds, defaultAlgorithm, defaultArgs, algorithms, args);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.service.autofill.IAutofillFieldClassificationService sDefaultImpl;
    }
    static final int TRANSACTION_calculateScores = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.service.autofill.IAutofillFieldClassificationService impl) {
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
    public static android.service.autofill.IAutofillFieldClassificationService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  public void calculateScores(android.os.RemoteCallback callback, java.util.List<android.view.autofill.AutofillValue> actualValues, java.lang.String[] userDataValues, java.lang.String[] categoryIds, java.lang.String defaultAlgorithm, android.os.Bundle defaultArgs, java.util.Map algorithms, java.util.Map args) throws android.os.RemoteException;
}
