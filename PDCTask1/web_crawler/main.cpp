
#include "global.h"

bool init_conf()
{
	std::ifstream in("conf.conf");
	if (in.fail())
		return false;
	int flg = 0;
	while (!in.eof()) {
		std::string s;
		in >> s;
		if (s[0] != '#'){
			if (s == "N_THREADS") { in >> s; N_THREADS = atoi(s.c_str()); flg++; }
			if (s == "Q_D_MIN") { in >> s; Q_D_MIN = atoi(s.c_str()); flg++; }
			if (s == "MAX_PARSE_THREAD") { in >> s; MAX_PARSE_THREAD = atoi(s.c_str()); flg++; }
			if (s == "Q_D_MAX") { in >> s; Q_D_MAX = atoi(s.c_str()); flg++; }
			if (s == "Q_P_MIN") { in >> s; Q_P_MIN = atoi(s.c_str()); flg++; }
			if (s == "Q_P_MAX") { in >> s; Q_P_MAX = atoi(s.c_str()); flg++; }
			if (s == "HASH_SIZE") { in >> s; HASH_SIZE = atoi(s.c_str()); flg++; }
			if (s == "TIMEOUT") { in >> s; TIMEOUT = atoi(s.c_str()); flg++; }
		}
	}
	if (flg == 8)
		return true;
	return false;
}

void thread_func(bool flg, const unsigned int id)
{
	if(DEBUGFLAG) std::printf("[%d] start\n", id);
	bool status_flag = flg;
	downloader loader(TIMEOUT);
	parser pars;
	while(!stopflag) {
		// std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if(DEBUGFLAG) std::printf("[%d] pq size: %d, dq size: %d\n",
								  id, (int)q_pars_size, (int)q_download_size);

		//условия остановки потоков
		if (n_save >= MAX_DOWNLOAD_PAGES)
			stopflag = true;
		//если очереди пусты, нужна гарантия что все потоки закончили добавления
		if (save_only && id == 0 && q_download_size == 0 && q_pars_size == 0) {
			pause_count = 1;
			pauseflag = true;
			if(DEBUGFLAG) std::printf("[%d] IN PAUSE\n", id);
			while(pause_count < N_THREADS);//ждем пока все потоки не закончат работать
			if (q_download_size == 0 && q_pars_size == 0)
				stopflag = true;
			pauseflag = false;
		}
		if (pauseflag) {
			pause_count++;//поток сообщает, что закончил работу
			if(DEBUGFLAG) std::printf("[%d] pause count %d\n", id, (int)pause_count);
			while(pauseflag);
		}

		//если в очереди накопилось достаточно ссылок, искать новые не надо
		if (!save_only && q_download_size >= MAX_DOWNLOAD_PAGES - n_save)
			save_only = true;

		//определение специализации потока
		if(!status_flag && ((q_pars_size <= Q_P_MIN && q_download_size > Q_D_MIN)
			|| (q_download_size >= Q_D_MAX && q_pars_size < Q_P_MAX)
			)) {
			status_flag = true;
			if(DEBUGFLAG) std::printf("[%d] is downloader\n", id);
		}
		if (status_flag && ((q_pars_size > Q_P_MIN && id < MAX_PARSE_THREAD)
			|| (q_download_size <= Q_D_MIN && q_pars_size > Q_P_MIN)
			|| (n_save + q_pars_size >= MAX_DOWNLOAD_PAGES)
			|| (q_pars_size >= Q_P_MAX && q_download_size < Q_D_MAX)
			)) {//!!!!!
			status_flag = false;
			if(DEBUGFLAG) std::printf("[%d] is parser\n", id);
		}

		if (status_flag) {
			//status - downloader
			Tpair_s_ui* pair_url = NULL;
			if (!q_download.pop(pair_url)) {
				if(DEBUGFLAG) std::printf("[%d]d the download queue is empty\n", id);
				continue;
			}
			if(DEBUGFLAG) std::printf("[%d]d poped \"%s\"\n", id, pair_url->first.get()->c_str());
			q_download_size--;
			if (pair_url->second > MAX_DEPTH_GRAPH) {
				continue;
			}
			std::shared_ptr<std::string> content;
			content = loader.download(pair_url->first);
			if(*content == ""){
				if(DEBUGFLAG) std::printf("[%d]d page \"%s\" not download\n",
										  id, pair_url->first.get()->c_str());
				continue;
			}
			if(DEBUGFLAG) std::printf("[%d]d download link \"%s\"\n",
									  id, pair_url->first.get()->c_str());

			Tpair_s_ui* pair_content = new Tpair_s_ui;
			*pair_content = std::make_pair(content, pair_url->second);
			if (!q_pars.push(pair_content)) {
				if(DEBUGFLAG) std::printf("[%d]d content \"%s\" not push in queue\n",
										  id, pair_url->first.get()->c_str());
				continue;
			}
			q_pars_size++;
			if(DEBUGFLAG) std::printf("[%d]d pushed \"%s\"\n", id, pair_url->first.get()->c_str());

			delete pair_url;
		} else {
			//status - parser
			Tpair_s_ui* pair_content = NULL;
			if (!q_pars.pop(pair_content)){
				if(DEBUGFLAG) std::printf("[%d]d the parse queue is empty\n", id);
				continue;
			}
			q_pars_size--;
			if(DEBUGFLAG) std::printf("[%d]p poped \n", id);

			if (!save_only && pair_content->second >= MAX_DEPTH_GRAPH)
				save_only = true;
			if (save_only && pair_content->second < MAX_DEPTH_GRAPH
				&& q_download_size < MAX_DOWNLOAD_PAGES - n_save)
				save_only = true;
			if (!save_only) {
				if (pars.pars(pair_content->first)) {
					if(DEBUGFLAG) std::printf("[%d]d parse failed\n", id);
					continue;	
				}

				size_t n_url = pars.get_size();
				for (size_t i = 0; i < n_url; ++i) {
					std::shared_ptr<std::string> url;
					url = pars.get_url(i);
					if(table_reps.insert(*url.get())) {
						Tpair_s_ui* pair_url = new Tpair_s_ui;
						*pair_url = std::make_pair(url, pair_content->second + 1);
						if (!q_download.push(pair_url)) {
							if(DEBUGFLAG) std::printf("[%d]d link \"%s\" not push in queue\n",
													  id, url.get()->c_str());
							continue;
						}
						q_download_size++;
						if(DEBUGFLAG) std::printf("[%d]p pushed \"%s\"\n", id, url->c_str());
					} else {
						if(DEBUGFLAG) std::printf("[%d]p collision \"%s\"\n", id, url->c_str());
					}
				}
			}
			if (n_save < MAX_DOWNLOAD_PAGES) {
				pars.content_to_file(PATH_CONTENTS + "num" + std::to_string(n_save)
									+ "dp" + std::to_string(pair_content->second) +
									 "id" + std::to_string(id) + ".html", pair_content->first);
				n_save++;
				if(DEBUGFLAG) std::printf("[%d]p save(%d)\n", id, (int)n_save);
			}
			delete pair_content;
		}
			
	}
}

int main(int argc, const char* argv[])
{
	setbuf(stdout, NULL);

	if (argc < 5) {
		std::cerr << "ERROR: main args not found" << std::endl;
		return 1;
	}
    std::string url =  argv[1];
   	MAX_DEPTH_GRAPH = atoi(argv[2]);
	MAX_DOWNLOAD_PAGES = atoi(argv[3]);
	PATH_CONTENTS = argv[4];
	DEBUGFLAG = false;
	DEBUGFLAG = argc == 6 ? atoi(argv[5]) : false;

    if (!init_conf())
    	std::cerr << "ERROR: init conf" << std::endl;

    //init download queue
  	Tpair_s_ui* node = new Tpair_s_ui;
   	node->first = std::make_shared<std::string>(url);
   	node->second = 0;
   	if (!q_download.push(node)) {
   		std::cerr << "ERROR: link \""
						<< url << "\" not push in queue" << std::endl;
		return 1;
   	}

   	//init hashtab
   	table_reps.reset(HASH_SIZE);
   	table_reps.insert(url);
   	
	n_parse_thread = 1;
	n_download_thread = N_THREADS - 1;
	q_download_size = 1;
	q_pars_size = 0;
	n_save = 0;
	stopflag = false;
	save_only = false;
	pauseflag = false;

	std::thread threads[N_THREADS - 1];

	time_start = std::chrono::high_resolution_clock::now();
	for (unsigned int i = 0; i < N_THREADS - 1; ++i) {
		threads[i] = std::thread(&thread_func, true, i);
	}
	thread_func(false, N_THREADS - 1);

	std::for_each(threads, threads + N_THREADS - 1, std::mem_fn(&std::thread::join));
	time_end = std::chrono::high_resolution_clock::now();

	std::printf("parse q size: %d, download q size: %d, save pages %d\n",
							  (int)q_pars_size, (int)q_download_size, (int)n_save);
	std::cout << "time: "<< std::chrono::duration_cast<std::chrono::milliseconds>
    			((time_end - time_start)).count() << "ms" << std::endl;

  	return 0;

}
