/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net.ipmemorystore;
/** {@hide} */
public class SameL3NetworkResponseParcelable implements android.os.Parcelable
{

  public java.lang.String l2Key1;

  public java.lang.String l2Key2;

  public float confidence;
  public static final android.os.Parcelable.Creator<SameL3NetworkResponseParcelable> CREATOR = new android.os.Parcelable.Creator<SameL3NetworkResponseParcelable>() {
    @Override
    public SameL3NetworkResponseParcelable createFromParcel(android.os.Parcel _aidl_source) {
      SameL3NetworkResponseParcelable _aidl_out = new SameL3NetworkResponseParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public SameL3NetworkResponseParcelable[] newArray(int _aidl_size) {
      return new SameL3NetworkResponseParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    _aidl_parcel.writeString(l2Key1);
    _aidl_parcel.writeString(l2Key2);
    _aidl_parcel.writeFloat(confidence);
    int _aidl_end_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.setDataPosition(_aidl_start_pos);
    _aidl_parcel.writeInt(_aidl_end_pos - _aidl_start_pos);
    _aidl_parcel.setDataPosition(_aidl_end_pos);
  }
  public final void readFromParcel(android.os.Parcel _aidl_parcel)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    int _aidl_parcelable_size = _aidl_parcel.readInt();
    if (_aidl_parcelable_size < 0) return;
    try {
      l2Key1 = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      l2Key2 = _aidl_parcel.readString();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      confidence = _aidl_parcel.readFloat();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
    } finally {
      _aidl_parcel.setDataPosition(_aidl_start_pos + _aidl_parcelable_size);
    }
  }
  @Override public int describeContents()
  {
    return 0;
  }
}
