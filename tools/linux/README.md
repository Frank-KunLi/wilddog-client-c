#tools/linux
##1.�ļ��ṹ��˵��

	
    	������ autotest.sh
    	������ cov.sh
    	������ uncov.sh
    

*	`autotest.sh` : �Զ����Խű���ʹ�÷�ʽ: `autotest.sh nosec|tinydtls|dtls`��nosec|tinydtls|dtls��ѡһ
*	`cov.sh` : �����ʲ��Խű���ʹ�÷�ʽ: `cov.sh nosec|tinydtls|dtls`��nosec|tinydtls|dtls��ѡһ
*	`uncov.sh`�������ʲ�������ű���ʹ�÷�ʽ: `cov.sh nosec|tinydtls|dtls`��nosec|tinydtls|dtls��ѡһ

##2.����˵��
ÿ�����������Ҫ���ƶ˽��������޸Ĳ���ȡ�Բ�����׼ȷ�Ժ��ȶ��ԡ��û������޸�`test_config.h`���ò���ʹ�õ�URL��

- `TEST_URL` �� ����ʱʹ�õ�URL��
- `TEST_URL2` �� ���ƶ˲�������һ��URL��
- `TEST_URL3` �� ���ƶ˲�������һ��URL��
- `TEST_AUTH` �� ��`TEST_URL`�����Ự��Auth��

##3.ʹ�ò���
1. ����`test_config.h`��ȷ������ʱʹ�õ��ƶ�URL��
2. ����SDK�Ķ���Ŀ¼���޸�`tests/linux/test_config.h`,ִ�в��Խű���
    $ cd tools/linux
	$ ./xxxx.sh nosec

3. �ȴ��������ն˿������Խ����
