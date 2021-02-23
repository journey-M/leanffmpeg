package gwj.dev.ffmpeg.videoEdit.material;

import gwj.dev.ffmpeg.videoEdit.VideoAPI;

public class VideoMaterial extends Material {

  private String vfilepath;

  @Override
  Type getMateType() {
    return Type.TYPE_VIDEO;
  }

  public VideoMaterial(String vfilepath) {
    this.vfilepath = vfilepath;
  }

  public void create() {
    VideoAPI videoAPI = new VideoAPI();
    //videoAPI.createThumbs(vfilepath);
  }
}
