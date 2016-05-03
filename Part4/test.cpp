#include <iostream>
#include <utility>
#include <sstream>
#include <queue>
using namespace std;





int main(){
	queue<pair<istringstream*,int>> q;
	string t = "test";
	istringstream iss(t);
	auto p = make_pair(&iss,5);
	q.push(p);

}