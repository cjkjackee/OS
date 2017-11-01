#include <iostream>
using namespace std;

int main ()
{
    int n;
    int t;
    int tim =0;
    double wait=0;
    double turn=0;
    int i=0;

    cin >> n;
    cout << "Process   Waiting Time   Turnaround Time" << endl;
    while(cin>>t)
    {
        cout<< "P[" << ++i << "]";
        cout.width(6);
       cout << left << " ";
       cout.width(15);
       cout << left << tim;
       wait += tim;
       tim += t;
       turn += tim;
       cout << tim << endl;
    }

    cout.precision(7);
    cout << endl;
    cout << "Average waiting time :" << fixed << wait/i << endl;
    cout << "Average turnaround time :" << fixed << turn/i << endl;

    return 0;
}
