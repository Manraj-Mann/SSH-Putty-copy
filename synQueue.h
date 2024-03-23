#include <queue>
#include <mutex>
#include <iostream>


class syncQueue{

	std::queue<char *> queue;
	std::mutex mtx; // Mutex for critical section
	public:

	void push( char * value){

		std::lock_guard<std::mutex> lock(mtx);
		queue.push(value);
        // std::cout<<queue.front()<<std::endl;
	}

	char * pop(){

		std::lock_guard<std::mutex> lock(mtx);
		if(queue.empty()){

			return "";
		}
		char * element = queue.front();
		queue.pop();
		return element;
		
	}

	bool empty(){

		std::lock_guard<std::mutex> lock(mtx);
		return queue.empty();
	}

};