/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.view;
/**
 * Listener for changes to the system gesture exclusion region
 *
 * {@hide}
 */
public interface ISystemGestureExclusionListener extends android.os.IInterface
{
  /** Default implementation for ISystemGestureExclusionListener. */
  public static class Default implements android.view.ISystemGestureExclusionListener
  {
    /**
         * Called when the system gesture exclusion for the given display changed.
         * @param displayId the display whose system gesture exclusion changed
         * @param systemGestureExclusion a {@code Region} where the app would like priority over the
         *                               system gestures, in display coordinates. Certain restrictions
         *                               might be applied such that apps don't get all the exclusions
         *                               they request.
         * @param systemGestureExclusionUnrestricted a {@code Region} where the app would like priority
         *                               over the system gestures, in display coordinates, without
         *                               any restrictions applied. Null if no restrictions have been
         *                               applied.
         */
    @Override public void onSystemGestureExclusionChanged(int displayId, android.graphics.Region systemGestureExclusion, android.graphics.Region systemGestureExclusionUnrestricted) throws android.os.RemoteException
    {
    }
    @Override
    public android.os.IBinder asBinder() {
      return null;
    }
  }
  /** Local-side IPC implementation stub class. */
  public static abstract class Stub extends android.os.Binder implements android.view.ISystemGestureExclusionListener
  {
    private static final java.lang.String DESCRIPTOR = "android.view.ISystemGestureExclusionListener";
    /** Construct the stub at attach it to the interface. */
    public Stub()
    {
      this.attachInterface(this, DESCRIPTOR);
    }
    /**
     * Cast an IBinder object into an android.view.ISystemGestureExclusionListener interface,
     * generating a proxy if needed.
     */
    public static android.view.ISystemGestureExclusionListener asInterface(android.os.IBinder obj)
    {
      if ((obj==null)) {
        return null;
      }
      android.os.IInterface iin = obj.queryLocalInterface(DESCRIPTOR);
      if (((iin!=null)&&(iin instanceof android.view.ISystemGestureExclusionListener))) {
        return ((android.view.ISystemGestureExclusionListener)iin);
      }
      return new android.view.ISystemGestureExclusionListener.Stub.Proxy(obj);
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
        case TRANSACTION_onSystemGestureExclusionChanged:
        {
          data.enforceInterface(descriptor);
          int _arg0;
          _arg0 = data.readInt();
          android.graphics.Region _arg1;
          if ((0!=data.readInt())) {
            _arg1 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg1 = null;
          }
          android.graphics.Region _arg2;
          if ((0!=data.readInt())) {
            _arg2 = android.graphics.Region.CREATOR.createFromParcel(data);
          }
          else {
            _arg2 = null;
          }
          this.onSystemGestureExclusionChanged(_arg0, _arg1, _arg2);
          return true;
        }
        default:
        {
          return super.onTransact(code, data, reply, flags);
        }
      }
    }
    private static class Proxy implements android.view.ISystemGestureExclusionListener
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
           * Called when the system gesture exclusion for the given display changed.
           * @param displayId the display whose system gesture exclusion changed
           * @param systemGestureExclusion a {@code Region} where the app would like priority over the
           *                               system gestures, in display coordinates. Certain restrictions
           *                               might be applied such that apps don't get all the exclusions
           *                               they request.
           * @param systemGestureExclusionUnrestricted a {@code Region} where the app would like priority
           *                               over the system gestures, in display coordinates, without
           *                               any restrictions applied. Null if no restrictions have been
           *                               applied.
           */
      @Override public void onSystemGestureExclusionChanged(int displayId, android.graphics.Region systemGestureExclusion, android.graphics.Region systemGestureExclusionUnrestricted) throws android.os.RemoteException
      {
        android.os.Parcel _data = android.os.Parcel.obtain();
        try {
          _data.writeInterfaceToken(DESCRIPTOR);
          _data.writeInt(displayId);
          if ((systemGestureExclusion!=null)) {
            _data.writeInt(1);
            systemGestureExclusion.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          if ((systemGestureExclusionUnrestricted!=null)) {
            _data.writeInt(1);
            systemGestureExclusionUnrestricted.writeToParcel(_data, 0);
          }
          else {
            _data.writeInt(0);
          }
          boolean _status = mRemote.transact(Stub.TRANSACTION_onSystemGestureExclusionChanged, _data, null, android.os.IBinder.FLAG_ONEWAY);
          if (!_status && getDefaultImpl() != null) {
            getDefaultImpl().onSystemGestureExclusionChanged(displayId, systemGestureExclusion, systemGestureExclusionUnrestricted);
            return;
          }
        }
        finally {
          _data.recycle();
        }
      }
      public static android.view.ISystemGestureExclusionListener sDefaultImpl;
    }
    static final int TRANSACTION_onSystemGestureExclusionChanged = (android.os.IBinder.FIRST_CALL_TRANSACTION + 0);
    public static boolean setDefaultImpl(android.view.ISystemGestureExclusionListener impl) {
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
    public static android.view.ISystemGestureExclusionListener getDefaultImpl() {
      return Stub.Proxy.sDefaultImpl;
    }
  }
  /**
       * Called when the system gesture exclusion for the given display changed.
       * @param displayId the display whose system gesture exclusion changed
       * @param systemGestureExclusion a {@code Region} where the app would like priority over the
       *                               system gestures, in display coordinates. Certain restrictions
       *                               might be applied such that apps don't get all the exclusions
       *                               they request.
       * @param systemGestureExclusionUnrestricted a {@code Region} where the app would like priority
       *                               over the system gestures, in display coordinates, without
       *                               any restrictions applied. Null if no restrictions have been
       *                               applied.
       */
  public void onSystemGestureExclusionChanged(int displayId, android.graphics.Region systemGestureExclusion, android.graphics.Region systemGestureExclusionUnrestricted) throws android.os.RemoteException;
}
