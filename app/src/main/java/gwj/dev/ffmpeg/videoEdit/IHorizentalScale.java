package gwj.dev.ffmpeg.videoEdit;

public interface IHorizentalScale {
  int SINGLE_WIDTH = 120;


  void setTimeLength(int len);

  void onHorizentalScal(long id, float porcent);
}
