package band_img_download;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;


/**
 * 
 * ������!!!
 * band ���� ������ ȭ��ũ�� �Ǵ� �ִ� ������ �ǽð����� �ε��Ѵ�.
 * �׷��� ��� ������ thumbnail img �ּҸ� ã�� �� ����.
 * ��� �������� �ε� �� �� �ִ� �Ķ���Ͱ� �ִ��� ���� !!!
 * ���� ������ �ε��ϴ� �ҽ��� ����� .... ������ �м��� ��Ʊ��� ... �ڹٽ�ũ��Ʈ.. �Ƚ�
 * 
 * @author kinow
 *
 */
public class NaverBandImgDonwload {

	private String file;
	private URL url;
	private BufferedReader pageReader;
	
	public NaverBandImgDonwload(String page)
	{
		this.file = page;
	}
	
	public NaverBandImgDonwload(URL url)
	{
		this.url = url;
	}
	
	static public String extract(StringBuffer sb)
	{
		String line;
		/*
		 * <img class="thumbnail _image" 
		 * src="https://coresos-phinf.pstatic.net/a/2if001/g_529Ud018svc1lckpxz2r5oti_n33kbp.jpg?type=s276" 
		 * alt="" style="display: inline;">
		 */
		//System.out.println(">>> "+sb.toString().length()+" / "+sb.toString());
		
		int idx;
		final String match1 = "<img class=\"thumbnail _image\" src=\"";
		final String match2 = "?type=s276";
		
		idx = sb.indexOf(match1);
		if (idx != -1) {
			//System.out.println(">>>>>> match1: "+idx +" "+sb.charAt(idx)+sb.charAt(idx+1)+sb.charAt(idx+2));
			//System.out.println("...."+sb.toString());
			sb.delete(0, match1.length() + idx);
			//System.out.println("...."+sb.toString());
			idx = sb.indexOf(match2);
			if (idx != -1) {
				//System.out.println("0 ~ "+idx + "="+sb.substring(0, idx));
				line = sb.substring(0, idx);
				sb.delete(0, idx+match2.length());
				return line;
			}
		}
		sb.delete(0, match1.length());
		return null;
	}
	
	public long download() throws Exception
	{
		String line;
		String downloadUrl;
		StringBuffer sb;
		long matched = 0;
		
		if (this.pageReader == null) {
			throw new Exception("unknown target");
		}
		
		while ((line = this.pageReader.readLine()) != null) {
			sb = new StringBuffer(line);
			while ((downloadUrl = this.extract(sb)) != null) {
				System.out.println(downloadUrl);
				matched++;
			}
		}
		return matched;
	}
	
	public boolean loadPage()
	{
		InputStream is = null;
		FileReader fr = null;
		
		try {
			if (url != null) {
				is = url.openStream();
				this.pageReader = new BufferedReader(new InputStreamReader(is));
			} else {
				fr = new FileReader(this.file);
				this.pageReader = new BufferedReader(fr);
			}
		} catch (Exception e) {
			e.printStackTrace();
			try {
				if (is != null) is.close();
				if (fr != null) fr.close();
			} catch (Exception ee) {}			
			return false;
		}
		return true;
	}
	
	private void close()
	{
		if (this.pageReader != null) {
			try {
				this.pageReader.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
	
	public static void main(String[] args) {
		long totalDownload = 0;
		//String target = "http://band.us/band/63415399/album/33547337";
		String target = "d:\\band.html";
		NaverBandImgDonwload band = new NaverBandImgDonwload(target);
		
		try {
			band.loadPage();
			totalDownload = band.download();
		} catch (Exception e) {
			e.printStackTrace();
		}
		System.out.println("totalDownload = "+totalDownload);
	}

}
