/*
* Последовательная версия программы
*/
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

void thread_func()
{
    int id = 0;
    if(DEBUGFLAG) std::printf("[%d] start\n", id);
   // bool status_flag = true;
    downloader loader(TIMEOUT);
    parser pars;
    while(!stopflag) {
        if(DEBUGFLAG) std::printf("[%d] pq size: %d, dq size: %d\n", id, (int)q_pars_size, (int)q_download_size);

        if (n_save >= MAX_DOWNLOAD_PAGES) 
            stopflag = true;
        if (save_only && q_download_size == 0 && q_pars_size == 0) {
                stopflag = true;
        }
        if (!save_only && q_download_size >= MAX_DOWNLOAD_PAGES - n_save)
            save_only = true;

        //status - downloader
        {
        Tpair_s_ui* pair_url = NULL;
        if (!q_download.pop(pair_url)) {
            std::cerr << "ERROR: failed to retrieve an item from the download queue" << std::endl;
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
            std::cerr << "ERROR: page \"" << *pair_url->first << "\" not download" << std::endl;
            continue;
        }
        if(DEBUGFLAG) std::printf("[%d]d download link \"%s\"\n", id, pair_url->first.get()->c_str());

        Tpair_s_ui* pair_content = new Tpair_s_ui;
        *pair_content = std::make_pair(content, pair_url->second);
        if (!q_pars.push(pair_content)) {
            std::cerr << "ERROR: content \"" << *pair_url->first << "\" not push in queue" << std::endl;
            continue;
        }
        q_pars_size++;
        if(DEBUGFLAG) std::printf("[%d]d pushed \"%s\"\n", id, pair_url->first.get()->c_str());

        delete pair_url;
        }
        //status - parser
        {
        Tpair_s_ui* pair_content = NULL;
        if (!q_pars.pop(pair_content)){
            std::cerr << "ERROR: failed to retrieve an item from the parse queue" << std::endl;
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
                std::cerr << "ERROR: parse failed" << std::endl;
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
                        std::cerr << "ERROR: link \"" 
                            << *url << "\" not push in queue" << std::endl;
                        continue;
                    }
                    q_download_size++;
                    if(DEBUGFLAG) std::printf("[%d]p pushed \"%s\"\n", id, url->c_str());
                } else {
                    if(DEBUGFLAG) std::printf("[%d]p collision \"%s\"\n", id, url->c_str());
                }
            }
        }
        if (!stopflag) {
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
    
    q_download_size = 1;
    q_pars_size = 0;
    n_save = 0;
    stopflag = false;
    save_only = false;

    time_start = std::chrono::high_resolution_clock::now();
    thread_func();
    time_end = std::chrono::high_resolution_clock::now();

    if(DEBUGFLAG) std::printf("pq size: %d, dq size: %d\n", (int)q_pars_size, (int)q_download_size);
    std::cout << "time: "<< std::chrono::duration_cast<std::chrono::milliseconds>
                ((time_end - time_start)).count() << "ms" << std::endl;


    return 0;        

}
