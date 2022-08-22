#include "public.h"

/**
 * @brief ���������գ��������ڼ�
 * @param year
 * @param month
 * @param day
 * @return return value: 0      1      2      3     4       5     6 
 *                 week: һ     ��     ��     ��     ��     ��     ��
 * @note ���չ�ʽ: https://baike.baidu.com/item/%E8%94%A1%E5%8B%92%E5%85%AC%E5%BC%8F/10491767
 */
int get_Week(int y, int m, int d)
{
    int week = 0;
    if(m==1 || m == 2)
    {
        m += 12;
        y--;
    }
    week = (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;
    return week;
}
