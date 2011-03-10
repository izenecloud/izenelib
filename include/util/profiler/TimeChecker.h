#ifndef _SF1LIB_TIME_CHECKER__
#define _SF1LIB_TIME_CHECKER__

#include <util/ProcMemInfo.h>
NS_IZENELIB_UTIL_BEGIN

#include <iostream>
using namespace std;

#if defined(WIN32)
#   ifndef _WINSOCKAPI_
#       define _WINSOCKAPI_
#   endif
#endif

#if defined(WIN32)
#	include <windows.h>
#else
#	include <sys/time.h>
#endif

// Ÿ�̸� Ŭ����
// ���� �ð��� ~�� �ð����� ���̸� ��ȯ�Ѵ�. ���� üũ�� ���ȴ�.
class TimeChecker {
public:
	// ����
	TimeChecker();
	// �Ҹ���
	virtual ~TimeChecker();

	/**
	 * ���� �ð� ��; ��d�Ѵ�.
	 *
	 * @param void
	 *
	 * @return void
	 **/
	void begin(void);

	/**
	 * ~�� �ð� ��; ��d�Ѵ�.
	 *
	 * @param void
	 *
	 * @return void
	 **/
	void end(void);

	/**
	 * ���� �ð��� ~�� �ð��� ���̸� ��ȯ�Ѵ�.
	 *
	 * �� �Լ� ȣ���ϱ� �� start(), end()�� ȣ��Ǿ�� �Ѵ�.
	 *
	 * @param void
	 *
	 * @return start�� end�� �ð����̸� ��ȯ (��': ��)
	 **/
	double diff(void);

	unsigned long vm_diff(void);
	unsigned long rss_diff(void);

private:
#if defined(WIN32)
	DWORD begin_; // ���� �ð�
	DWORD end_; // ~�� �ð�
#else
	struct timeval begin_; // ���� �ð�
	struct timeval end_; // ~�� �ð�
#endif
	unsigned long svm_, srss_;
	unsigned long evm_, erss_;

	void getMemInfo_(unsigned long &vm, unsigned long &rss) {		
			ProcMemInfo::getProcMemInfo(vm, rss);
	}
};

class TimeChecker1 {
public:
	// ����
	TimeChecker1();
	// �Ҹ���
	virtual ~TimeChecker1();

	/**
	 * ���� �ð� ��; ��d�Ѵ�.
	 *
	 * @param void
	 *
	 * @return void
	 **/
	void begin(void);

	/**
	 * ~�� �ð� ��; ��d�Ѵ�.
	 *
	 * @param void
	 *
	 * @return void
	 **/
	void end(void);

	/**
	 * ���� �ð��� ~�� �ð��� ���̸� ��ȯ�Ѵ�.
	 *
	 * �� �Լ� ȣ���ϱ� �� start(), end()�� ȣ��Ǿ�� �Ѵ�.
	 *
	 * @param void
	 *
	 * @return start�� end�� �ð����̸� ��ȯ (��': ��)
	 **/
	double diff(void);

	unsigned long vm_diff(void);
	unsigned long rss_diff(void);

private:
#if defined(WIN32)
	DWORD begin_; // ���� �ð�
	DWORD end_; // ~�� �ð�
#else
	struct timeval begin_; // ���� �ð�
	struct timeval end_; // ~�� �ð�
#endif
	unsigned long svm_, srss_;
	unsigned long evm_, erss_;

	void getMemInfo_(unsigned long &vm, unsigned long &rss) {		
	}
};

NS_IZENELIB_UTIL_END //namespace wiselib

#endif // _SF1LIB_TIME_CHECKER__
