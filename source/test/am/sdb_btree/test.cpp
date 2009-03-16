#include <iostream>
#include <cstdlib>

using namespace std;

struct tt
{
  static int t;
};

int tt::t;


int main()
{
cout<<sizeof(tt)<<endl;
cout<<sizeof(long)<<endl;
int a[5] = {1,2,3,4,5};
memcpy(&a[2], &a[0], sizeof(int)*3);
for(int i=0; i<5; i++)
   cout<<a[i]<<endl;

return 1;
}
