/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.app.job;
/**
 * Interface that the framework uses to communicate with application code that implements a
 * JobService.  End user code does not implement this interface directly; instead, the app's
 * service implementation will extend android.app.job.JobService.
 * {@hide}
 */
public interface IJobService extends android.os.IInterface
{
  /** Default implementation for IJobService. */
  public static class Default implements android.app.job.IJobService
  {
    /** Begin execution of application's job. */
    @Override public void startJob(android.app.job.JobParameters jobParams) throws android.os.RemoteException
    {
    }
    /** Stop execution of application's job. */
    @Override public void stopJob(android.app.job.JobParameters jobParams) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.app.job.IJobService
  {
    private static final java.lang.String DESCRIPTOR = "android.app.job.IJobService";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.app.job.IJobService interface,
     * generating a proxy if needed.
     */
    public static android.app.job.IJobService asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.app.job.IJobService))) {
        return ((android.app.job.IJobService)iin);
      }
      return new android.app.job.IJobService.Stub.Proxy(obj);
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
        case TRANSACTION_startJob:
        {
          data.enforceInterface(descriptor);
          android.app.job.JobParameters _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.job.JobParameters.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.startJob(_arg0);
          return true;
        }
        case TRANSACTION_stopJob:
        {
          data.enforceInterface(descriptor);
          android.app.job.JobParameters _arg0;
          if ((0!=data.readInt())) {
            _arg0 = android.app.job.JobParameters.CREATOR.createFromParcel(data);
          }
          else {
            _arg0 = null;
          }
          this.stopJob(_arg0);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.app.job.IJobService
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
      /** Begin execution of application's job. */
      @Override public void startJob(android.app.job.JobParameters jobParams) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((jobParams!=null)) {
            _data.writeInt(1);
            jobParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_startJob, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().startJob(jobParams);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      /** Stop execution of application's job. */
      @Override public void stopJob(android.app.job.JobParameters jobParams) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          if ((jobParams!=null)) {
            _data.writeInt(1);
            jobParams.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_stopJob, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().stopJob(jobParams);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.app.job.IJobService sDefaultImpl;
    }
    static final int TRANSACTION_startJob = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    static final int TRANSACTION_stopJob = (android.os.IBinder.FIRST_CALL_TRANSACTION + 1);
    public static boolean setDefaultImpl(android.app.job.IJobService impl) {
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
    public static android.app.job.IJobService getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /** Begin execution of application's job. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/job/IJobService.aidl:29:1:29:25")
  public void startJob(android.app.job.JobParameters jobParams) throws android.os.RemoteException;
  /** Stop execution of application's job. */
  @android.compat.annotation.UnsupportedAppUsage(overrideSourcePosition="/Volumes/1tdisk/Android10/android-10.0.0_r41/frameworks/base/core/java/android/app/job/IJobService.aidl:32:1:32:25")
  public void stopJob(android.app.job.JobParameters jobParams) throws android.os.RemoteException;
}
