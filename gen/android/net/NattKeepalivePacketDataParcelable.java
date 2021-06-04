/*___Generated_by_IDEA___*/

/*
 * This file is auto-generated.  DO NOT MODIFY.
 */
package android.net;
public class NattKeepalivePacketDataParcelable implements android.os.Parcelable
{

  public byte[] srcAddress;

  public int srcPort;

  public byte[] dstAddress;

  public int dstPort;
  public static final android.os.Parcelable.Creator<NattKeepalivePacketDataParcelable> CREATOR = new android.os.Parcelable.Creator<NattKeepalivePacketDataParcelable>() {
    @Override
    public NattKeepalivePacketDataParcelable createFromParcel(android.os.Parcel _aidl_source) {
      NattKeepalivePacketDataParcelable _aidl_out = new NattKeepalivePacketDataParcelable();
      _aidl_out.readFromParcel(_aidl_source);
      return _aidl_out;
    }
    @Override
    public NattKeepalivePacketDataParcelable[] newArray(int _aidl_size) {
      return new NattKeepalivePacketDataParcelable[_aidl_size];
    }
  };
  @Override public final void writeToParcel(android.os.Parcel _aidl_parcel, int _aidl_flag)
  {
    int _aidl_start_pos = _aidl_parcel.dataPosition();
    _aidl_parcel.writeInt(0);
    _aidl_parcel.writeByteArray(srcAddress);
    _aidl_parcel.writeInt(srcPort);
    _aidl_parcel.writeByteArray(dstAddress);
    _aidl_parcel.writeInt(dstPort);
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
      srcAddress = _aidl_parcel.createByteArray();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      srcPort = _aidl_parcel.readInt();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      dstAddress = _aidl_parcel.createByteArray();
      if (_aidl_parcel.dataPosition() - _aidl_start_pos >= _aidl_parcelable_size) return;
      dstPort = _aidl_parcel.readInt();
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
