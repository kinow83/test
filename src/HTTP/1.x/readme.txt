1. connection
	- HTTP 1.0: Connection: close (default)
	- HTTP 1.1: Connection: Keep-Alive (default) - persist
				Connection: close <- force close
	�̽�: Persistent Connection�� ������� �ʴ� ���
			������ ����� ��� Ŭ���̾�Ʈ�� TCP������ ��� �þ�ٺ���
			������ �ڿ��� ���Ǿ� ���� ���� Ŭ���̾�Ʈ�� ���ӿ� ��ó�� �� ���� ��Ȳ �ó�����
			���� Ŭ���̾�Ʈ�� ������ ���� ���� ���� �������� ���� URL������ 
			������ ���뼺�� ����Ͽ� Persistent Connection�� ������� ���...

			�ݴ�� Persistent connetion�� ����ϸ� ���� �� �ִ� ������
			������ ���� �ð��� TCP������ ���� �� ��ŭ ������. ���� CPU, �޸� �ڿ� ����,
			��Ʈ��ũ ȥ���̳� ������ ���� ���� ����.
2. pipelining
	- HTTP 1.1
	- �ټ��� HTTP ��û�� ������ Ŭ���̾�Ʈ�� ���� ���̿� ��������(HOL)����
		stream���·� �ְ� ���� �� �ִ� ���
	- ���� �޼���: GET, HEAD, PUT, DELETE
	�̽�: ���Ͻÿ� ������ �����ؾ� �ϴ� ����...

3. ������ ����
