#include"slfvector.h"
#include<iostream>

using namespace std;
using namespace izenelib::am::concurrent;

void test_push(){
    slfvector<int> vc;
    cout<<"Before push_back time: "<<time(NULL)<<endl;
    for(int i = 0; i < 100000000; ++i){
        vc.push_back(i);
    }
    cout<<"After 0.1 billion push_back: "<<time(NULL)<<endl;
}

void test_get(){
    slfvector<int> vc;
    for(int i = 0; i < 100000000; ++i){
        vc.push_back(i);
    }
    cout<<"Before get element time: "<<time(NULL)<<endl;
    for(int i = 0; i < 100000000; ++i){
        vc.at(i);
    }
    cout<<"After 0.1 billion at(): "<<time(NULL)<<endl;
}

void test_reserve(){
    slfvector<int> vc;
    for(int i = 0; i < 100000; ++i){
        vc.push_back(i);
    }
    vc.reserve(100);
    cout<<"Reserve: "<<vc.at(99)<<endl;
    cout<<"After reserved, visit will beyond visit: "<<endl;
    vc.at(100);
}

int main(){
    test_push();
    test_get();
    test_reserve();
}
