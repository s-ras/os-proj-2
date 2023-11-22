#include <windows.h>
#include <chrono>
#include <fstream>
#include <iostream>

using namespace std;

int DATA_COUNT;
float* dataset;
int TIME = -1;
float OPTIMAL[3] = {0};
int** answers;

struct inputParam {
	int index;
};

int lineCount(fstream&);
DWORD WINAPI Calc(void*);

int main() {
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int CORE_NUMBER = sysInfo.dwNumberOfProcessors;
	string FILE_NAME;
	fstream FILE;

	answers = new int*[CORE_NUMBER];

	do {
		cout << "Dataset file name?\t";
		cin >> FILE_NAME;
		FILE.open("dataset/" + FILE_NAME + ".txt", ios::in);
		if (!FILE) {
			cout << "File not found, try again!" << endl;
		}
	} while (!FILE);

	cout << "------------------------------------" << endl;

	do {
		cout << "Time in seconds?\t";
		cin >> TIME;
		if (TIME <= 0) {
			cout << "Invalid time, try again!" << endl;
		}
	} while (TIME <= 0);

	cout << "------------------------------------" << endl;

	DATA_COUNT = lineCount(FILE);

	dataset = new float[DATA_COUNT];

	for (int i = 0; i < CORE_NUMBER; i++) {
		answers[i] = new int[DATA_COUNT];
		for (int j = 0; j < DATA_COUNT; j++) {
			answers[i][j] = -1;
		}
	}

	cout << "Working on " << FILE_NAME << " with " << DATA_COUNT
		 << " records for " << TIME << " seconds:" << endl;

	for (int i = 0; i < DATA_COUNT; i++) {
		string read;
		getline(FILE, read, '\n');
		dataset[i] = stof(read);
		cout << dataset[i] << ' ';
	}

	cout << endl << "------------------------------------" << endl;

	cout << CORE_NUMBER << " CPU cores detected, launching " << CORE_NUMBER
		 << " threads" << endl;

	cout << "------------------------------------" << endl;

	float total = 0;

	for (int i = 0; i < DATA_COUNT; i++) {
		total += dataset[i];
	}

	OPTIMAL[0] = total * 2 / 5;
	OPTIMAL[1] = total * 2 / 5;
	OPTIMAL[2] = total * 1 / 5;

	inputParam threadParams[CORE_NUMBER];
	HANDLE threadHandle[CORE_NUMBER];

	for (int i = 0; i < CORE_NUMBER; i++) {
		threadParams[i].index = i;
		threadHandle[i] =
			CreateThread(NULL, 0, Calc, &threadParams[i], 0, NULL);
		if (!threadHandle[i]) {
			cout << "Could not create thread, program will terminate." << endl;
			exit(1);
		}
	}

	for (int i = 0; i < CORE_NUMBER; i++) {
		WaitForSingleObject(threadHandle[i], INFINITE);
		CloseHandle(threadHandle[i]);
	}

	int winnerIndex = -1;
	float diff = -1;

	for (int i = 0; i < CORE_NUMBER; i++) {
		float c1_sum = 0;
		float c2_sum = 0;
		float c3_sum = 0;

		for (int j = 0; j < DATA_COUNT; j++) {
			switch (answers[i][j]) {
				case 0:
					c1_sum += dataset[j];
					break;
				case 1:
					c2_sum += dataset[j];
					break;
				case 2:
					c3_sum += dataset[j];
					break;
				default:
					break;
			}
		}

		float newDiff = abs(OPTIMAL[0] - c1_sum) + abs(OPTIMAL[1] - c2_sum) +
						abs(OPTIMAL[2] - c3_sum);

		if (newDiff < diff || diff == -1) {
			diff = newDiff;
			winnerIndex = i;
		}
	}

	float firstChild[DATA_COUNT] = {-1};
	float secondChild[DATA_COUNT] = {-1};
	float thirdChild[DATA_COUNT] = {-1};
	int firstChildIndex = 0;
	int secondChildIndex = 0;
	int thirdChildIndex = 0;

	for (int i = 0; i < DATA_COUNT; i++) {
		switch (answers[winnerIndex][i]) {
			case 0:
				firstChild[firstChildIndex] = dataset[i];
				firstChildIndex++;
				break;
			case 1:
				secondChild[secondChildIndex] = dataset[i];
				secondChildIndex++;
				break;
			case 2:
				thirdChild[thirdChildIndex] = dataset[i];
				thirdChildIndex++;
				break;
			default:
				break;
		}
	}

	cout << "FIRST CHILD : " << endl;
	for (int i = 0; i < firstChildIndex; i++) {
		cout << firstChild[i] << ' ';
	}

	cout << endl << "+++++++++++" << endl;

	cout << "SECOND CHILD : " << endl;
	for (int i = 0; i < secondChildIndex; i++) {
		cout << secondChild[i] << ' ';
	}

	cout << endl << "+++++++++++" << endl;

	cout << "THIRD CHILD : " << endl;
	for (int i = 0; i < thirdChildIndex; i++) {
		cout << thirdChild[i] << ' ';
	}

	cout << endl << "------------------------------------" << endl;
	cout << "BEST DIFFERENCE : " << diff << endl;

	delete dataset;

	for (int i = 0; i < CORE_NUMBER; i++) {
		delete[] answers[i];
	}
	delete answers;
}

int lineCount(fstream& F) {
	int res = 0;
	string read;
	while (getline(F, read)) {
		res++;
	};
	F.clear();
	F.seekg(0, ios::beg);
	return res;
}

DWORD WINAPI Calc(void* param) {
	inputParam* inp = static_cast<inputParam*>(param);
	int index = inp->index;

	srand(time(0) + index);

	int res[DATA_COUNT] = {-1};
	float diff = -1;

	std::chrono::time_point startTime =
		std::chrono::high_resolution_clock::now();
	std::chrono::time_point currentTime = startTime;

	while (std::chrono::duration_cast<std::chrono::seconds>(currentTime -
															startTime)
			   .count() < TIME) {
		float c1_sum = 0;
		float c2_sum = 0;
		float c3_sum = 0;
		int tempRes[DATA_COUNT] = {-1};

		for (int i = 0; i < DATA_COUNT; i++) {
			int rnd = rand() % 3;
			switch (rnd) {
				case 0:
					tempRes[i] = 0;
					c1_sum += dataset[i];
					break;
				case 1:
					tempRes[i] = 1;
					c2_sum += dataset[i];
					break;
				case 2:
					tempRes[i] = 2;
					c3_sum += dataset[i];
					break;
				default:
					break;
			}
		}

		float newDiff = abs(OPTIMAL[0] - c1_sum) + abs(OPTIMAL[1] - c2_sum) +
						abs(OPTIMAL[2] - c3_sum);

		if (newDiff < diff || diff == -1) {
			diff = newDiff;
			for (int i = 0; i < DATA_COUNT; i++) {
				res[i] = tempRes[i];
			}
		}

		currentTime = std::chrono::high_resolution_clock::now();
	}

	for (int i = 0; i < DATA_COUNT; i++) {
		answers[index][i] = res[i];
	}

	return 0;
}