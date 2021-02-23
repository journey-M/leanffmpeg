package gwj.dev.ffmpeg.videoEdit.material;

public abstract class Material {
  public enum Type{
    TYPE_VIDEO,
    TYPE_AUDIO,
  }

  abstract Type getMateType();
}

