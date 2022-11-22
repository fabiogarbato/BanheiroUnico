/*
	CURITIBA 11/2022
	UNIVERSIDADE POSITIVO
	PROGRAMACAO CONCORRENTE/DESENVOLVIMENTO DE SISTEMAS
	
	TRABALHO 2
	- ADAPTACAO DO "PROBLEMA DO BANHEIRO UNICO" (VER DETALHES NA ESPECIFICACAO)
	
	TAREFA
	- COMPLETAR/COMPLEMENTAR AS FUNCOES "CAT" E "DOG" NO FINAL DESTE ARQUIVO
	
	REGRAS
	- VOCE PODE ADICIONAR CODIGO A VONTADE DESDE QUE SEJA A PARTIR DA LINHA COM O COMENTARIO "TODO"
	- VOCE PODE INCLUIR CABECALHOS A VONTADE
	- NADA DO QUE ESTA ESCRITO PODE SER APAGADO
	
	INFORMACOES IMPORTANTES
	- A ACAO "EATING" EH CRITICA, A ACAO "PLAYING" EH NAO-CRITICA
	- DEVE HAVER EXCLUSAO MUTUA ENTRE GATOS E CACHORROS NA AREA DE COMIDA
	- O NUMERO DE PETS NA AREA DE COMIDA NAO PODE ULTRAPASAR O VALOR DA MACRO "MAX_PETS"
	- NAO DEVE HAVER STARVATION DE GATOS OU CACHORROS
	
	DICAS
	- HA UMA CLASSE "SEMAFORO" DISPONIVEL PARA USO
	- LEMBRE-SE DE COMPILAR PARA C++11 (-std=c++11) OU SUPERIOR
	- A COMPREENSAO DO CODIGO EH PARTE DO TRABALHO
*/

#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <iostream>

class semaphore
{
	long count;
	std::mutex mtx;
	std::condition_variable cv;
	
	public:
	
	semaphore(long const c = 0) : count(c) {}
	
	semaphore(semaphore const &) = delete;
	semaphore(semaphore &&) = default;
	semaphore & operator=(semaphore const &) = delete;
	semaphore & operator=(semaphore &&) = default;
	~semaphore() = default;
	
	void acquire() //aka "wait", "down", "p"
	{
		auto lock = std::unique_lock<std::mutex>(mtx);
		while(!count) cv.wait(lock);
		--count;
	}
	
	void release() //aka "signal", "up", "v"
	{
		auto lock = std::unique_lock<std::mutex>(mtx);
		++count;
		cv.notify_one();
	}
};

#define MAX_PETS 16
#define MAX_SLEEP_US 4
#define NUM_THREADS 100

void do_stuff(int const id, char const * kind, char const * action)
{
	std::printf("pet #%d (%s) started %s...\n", id, kind, action);
	std::this_thread::sleep_for(std::chrono::microseconds(std::rand() % MAX_SLEEP_US));
	std::printf("pet #%d (%s) stopped %s...\n", id, kind, action);
}

void cat(int const);
void dog(int const);
int countDog = 0;
int countCat = 0;
semaphore semaforoCritico(1), semarofoAux(1);
std::mutex mCat, mDog;
int main()
{
	auto pets = std::vector<std::thread>(NUM_THREADS);
	
	for(int i = 0; i < pets.size(); ++i)
	{
		pets.at(i) = std::thread(i % 2 ? cat : dog, i);
	}
	
	for(int i = 0; i < pets.size(); ++i)
	{
		pets.at(i).join();
	}
	
	return 0;
}

//////////////////////////////////////////////////////////////
//TODO
//////////////////////////////////////////////////////////////

void cat(int const id)
{
	while(true)
	{
		do_stuff(id, "cat", "playing");

		semarofoAux.acquire();
		mCat.lock(); 

		if(countCat < MAX_PETS){
			countCat++;

			if(countCat == 1){
            	semaforoCritico.acquire();
        	}

			mCat.unlock();
			semarofoAux.release();
			
			do_stuff(id, "cat", "eating");

			mCat.lock();

			countCat--;

			if(countCat == 0){
				semaforoCritico.release();
			}

			mCat.unlock();
		
		} else {
			mCat.unlock();
			semarofoAux.release();
		}
		
	}
}

void dog(int const id)
{
	while(true)
	{

		do_stuff(id, "dog", "playing");

		semarofoAux.acquire();
		mDog.lock(); 

		if(countDog < MAX_PETS){
			countDog++;

			if(countDog == 1){
            	semaforoCritico.acquire();
        	}

			mDog.unlock();
			semarofoAux.release();
			
			do_stuff(id, "dog", "eating");

			mDog.lock();

			countDog--;

			if(countDog == 0){
				semaforoCritico.release();
			}

			mDog.unlock();
		
		} else {
			mDog.unlock();
			semarofoAux.release();
		}
	}
}

//semafaro gato cachorro ok
//contador pros dois ok
//chegou gato bloqueia cachorro
//ultima gato libera cachorro
//leitor
//talvez um aux

