import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;

public class HttpClient {

	public static void main(String[] args) {
		// TODO Auto-generated method stub

		String res = HttpClient.executePost("https://www.naver.com", "aaa=100");
		
		System.out.println(res);
	}

	public static String executePost(String targetURL, String urlParameters) {
		HttpURLConnection connection = null;

		try {
			// Create connection
			URL url = new URL(targetURL);
			connection = (HttpURLConnection) url.openConnection();
			
			if (!urlParameters.equals("")) {
				connection.setRequestMethod("POST");
				connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
				connection.setRequestProperty("Content-Length", Integer.toString(urlParameters.getBytes().length));
			} else {
				connection.setRequestMethod("GET");
			}
			connection.setRequestProperty("Content-Language", "en-US");

			connection.setUseCaches(false);
			connection.setDoOutput(true);

			if (!urlParameters.equals("")) {
				// Send request
				DataOutputStream wr = new DataOutputStream(connection.getOutputStream());
				wr.writeBytes(urlParameters);
				wr.close();
			}

			// Get Response
			InputStream is = connection.getInputStream();
			BufferedReader rd = new BufferedReader(new InputStreamReader(is));
			StringBuilder response = new StringBuilder(); // or StringBuffer if Java version 5+
			String line;
			while ((line = rd.readLine()) != null) {
				response.append(line);
				response.append('\r');
			}
			rd.close();
			return response.toString();
		} catch (Exception e) {
			e.printStackTrace();
			return null;
		} finally {
			if (connection != null) {
				connection.disconnect();
			}
		}
	}
}
