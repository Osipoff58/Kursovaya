/** @file
 * @author Осипов М.С.
 * @date 28.12.2023
 * @copyright ИБСТ ПГУ
 * @brief Модуль реализации методов класса WorkWithClient
 */
#include "ServKurs.h"

using namespace std;

WorkWithClient::WorkWithClient(const string bazep_adr, const string ferr_adr, const int n_port)
{
    if (n_port < 1025 or n_port > 65535) throw string("Ошибка выбора порта");
    string ptf, ptf2, bazep_adr_f, ferr_adr_f;

    ptf.resize(1024);
    auto ret = readlink("/proc/self/exe", &ptf[0], ptf.size());
    ptf.resize(ret);
    ptf2 = ptf;
    ptf.erase(ptf.find('/',2));
    ptf2.erase(ptf2.rfind('/'));

    if ((int)bazep_adr.find(ptf) == -1 and (int)bazep_adr.find('/') != -1) {
        bazep_adr_f = (string)ptf2 + bazep_adr;
    } else bazep_adr_f = bazep_adr;

    if ((int)ferr_adr.find(ptf) == -1 and (int)ferr_adr.find('/') != -1) {
        ferr_adr_f = (string)ptf2 + ferr_adr;
    } else ferr_adr_f = ferr_adr;

    ifstream bazep(bazep_adr_f);
    if (!(bazep.is_open())) throw string("Ошибка открытия файла базы клиентов");
    bazep.seekg(0,ios::end);
    int bazepsize = bazep.tellg();
    bazep.seekg(0,ios::beg);
    char* buf = new char[bazepsize];
    bazep.read(buf,bazepsize);
    if (!bazep) throw string("Ошибка чтения файла базы клиентов");
    allclients = string(buf);
    bazep.close();
    if ((int)allclients.find(':') == -1) throw string("Файл базы клиентов не содержит записей нужного формата");
    ferr.open(ferr_adr_f, ios::app);
    if (!(ferr.is_open())) {
        throw string("Ошибка открытия файла для записи ошибок");
    }
}



WorkWithClient::~WorkWithClient()
{
    ferr.close();
}



string WorkWithClient::inttohex(const uint64_t decnum)
{
    string ans = "";
    uint64_t cop = decnum;
    do {
        uint64_t r = cop%16;
        if (r > 9) {
            r += (uint64_t)'A' - 10;
        } else  (r += (uint64_t)'0');
        ans = (char)r + ans;
        cop /= 16;
    } while (cop != 0);
    return ans;
}



string WorkWithClient::makesalt()
{
    random_device seed;
    mt19937 rnd(seed());
    uniform_int_distribution<uint64_t> range(1,0xFFFFFFFFFFFFFFFF);
    uint64_t x;
    x = range(rnd);
    string str = inttohex(x);
    while (str.size() < 16) {
        str = '0' + str;
    }
    salt = str;
    return salt;
}



bool WorkWithClient::checID(const string newid)
{
    int poz = 0;
    while(true) {
        if ((int)allclients.find(newid, poz) == -1) {
            return false;
        } else {
            poz =(int)allclients.find(newid) + (int)newid.size();
            if (allclients[poz] != ':') {
                continue;
            }
            clientpar = "";
            for (int i = poz + 1 ; allclients[i] != '\n' and allclients[i+1] != '\0'; i++) {
                clientpar += allclients[i];
            }
            return true;
        }

    }
}



bool WorkWithClient::checparol(const string parol_to_chek)
{
    using namespace CryptoPP;
    Weak::MD5 hash;
    string truepar;

    StringSource(salt + clientpar, true,
                 new HashFilter(hash,
                                new HexEncoder(
                                    new StringSink(truepar))));

    if (truepar == parol_to_chek) {
        return true;
    } else {
        return false;
    }
}

uint64_t WorkWithClient::count(const uint32_t len, const uint64_t* data)
{
    if (len == 0) return 0;
    uint64_t rez = 0;
    for (uint32_t i = 0; i < len; ++i) {
        if (data[i] == 0) continue;
        if ((data[i] * data[i])/ data[i] == data[i]) {
            if (rez > rez + data[i]*data[i]) {
                rez = std::numeric_limits<uint64_t>::max();
                break;
            }
            rez += data[i]*data[i];
        } else {
            rez = std::numeric_limits<uint64_t>::max();
            break;
        }
    }
    return rez;
}



void WorkWithClient::errrecord(const string krit, const string reazon)
{
    locale loc("ru_RU.UTF-8");
    locale::global(loc);
    time_t err_times = time(NULL);
    tm* err_date_st;
    err_date_st= localtime(&err_times);
    char err_date_mass[80];
    std::strftime(err_date_mass,80,"%A %d %b %Y %T", err_date_st);
    string err_time_rez = string(err_date_mass);
    ferr << err_time_rez << "  " << krit << " – " << reazon << endl;
}
