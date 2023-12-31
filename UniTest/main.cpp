#include "ServKurs.h"
#include <string>
#include <vector>
#include <UnitTest++/UnitTest++.h>

SUITE(Server)// Проверка корректности введённых параметров
{
    TEST(NormTest) {   // Сценарий без ошибок
        WorkWithClient cp("/etc/vcalc.conf",
                          "/var/log/vcalc.log",
                          33333);
        CHECK(true);

    }
    TEST(WrongPort) {   // Недопустимый номер порта
        CHECK_THROW(WorkWithClient cp("/etc/vcalc.conf",
                                      "/home/stud/C++Projects/Lab1/Kurs/var/log/vcalc.log",
                                      1024),std::string);
        CHECK_THROW(WorkWithClient cp("/etc/vcalc.conf",
                                      "/var/log/vcalc.log",
                                      65536),std::string);
    }
    TEST(WrongPathBase) {   // Неверный путь к базе данных пользователей
        CHECK_THROW(WorkWithClient cp("/voo/bche/ne/put/a/dur/dom.conf",
                                      "/var/log/vcalc.log",
                                      33333),std::string);
    }
    TEST(BadBase) {   // Ошибки в содержании базы данных пользователей
        CHECK_THROW(WorkWithClient cp("/etc/bad_vcalc_1.conf",
                                      "/var/log/vcalc.log",
                                      33333),std::string);
    }
    TEST(WrongPathJornal) {   // Неверный путь к файлу записи для ошибок
        CHECK_THROW(WorkWithClient cp("/etc/vcalc.conf",
                                      "/etot/toj/ni/chem/ni/luch/che/1.log",
                                      33333),std::string);
    }
}
struct Start_fix {
    WorkWithClient * p;
    Start_fix()
    {
        p = new WorkWithClient("/etc/vcalc.conf",
                               "/var/log/vcalc.log",
                               33333);
    }
    ~Start_fix()
    {
        delete p;
    }
};
SUITE(IdentAndAut)// Проверка работы идентификации и аутентификации
{
    TEST_FIXTURE(Start_fix, GoodID) { // Присланное ID есть в базе
        CHECK_EQUAL(true,p->checID("user"));
    }
    TEST_FIXTURE(Start_fix, BadID) { // Присланное ID нет в базе
        CHECK_EQUAL(false,p->checID("Urser319"));
    }
    TEST_FIXTURE(Start_fix, GoodPr) { // Присланный пароль совпадает с паролем в базе
        p->checID("user");
        std::string salt = p->makesalt();
        std::string clientpar = "P@ssW0rd";
        using namespace CryptoPP;
        Weak::MD5 hash;
        std::string parl_to_chek;
        StringSource(salt + clientpar, true,
                     new HashFilter(hash,
                                    new HexEncoder(
                                        new StringSink(parl_to_chek))));
        CHECK_EQUAL(true,p->checparol(parl_to_chek));
    }
    TEST_FIXTURE(Start_fix, BadPr) { // Присланный пароль не совпадает с паролем в базе
        p->checID("user");
        std::string salt = p->makesalt();
        std::string clientpar = "Pass6W0rld99";
        using namespace CryptoPP;
        Weak::MD5 hash;
        std::string parl_to_chek;
        StringSource(salt + clientpar, true,
                     new HashFilter(hash,
                                    new HexEncoder(
                                        new StringSink(parl_to_chek))));
        CHECK_EQUAL(false,p->checparol(parl_to_chek));
    }
}

SUITE(Counting)//Проверка подсчёта результата
{
    TEST_FIXTURE(Start_fix, NormCout) { // Подсчёт без ошибок
        uint64_t* numbers {new uint64_t[4]{1, 2, 3, 4}};
        CHECK_EQUAL(30,p->count(4, numbers));
    }
    TEST_FIXTURE(Start_fix, LongVec) { // Присланная длина вектора меньше реальной
        uint64_t* numbers {new uint64_t[6]{1, 2, 3, 4, 5, 6}};
        CHECK_EQUAL(30,p->count(4, numbers));
    }
    TEST_FIXTURE(Start_fix, ShortVec) { // Присланная длина вектора больше реальной, неполученные значения принимают значение ноль
        uint64_t* numbers {new uint64_t[6]{1, 2, 3, 4}};
        CHECK_EQUAL(30,p->count(6, numbers));
    }
    TEST_FIXTURE(Start_fix, NullVec) { // Присланная длина равна нулю
        uint64_t* numbers {new uint64_t[4]{1, 2, 3, 4}};
        CHECK_EQUAL(0,p->count(0, numbers));
    }
    TEST_FIXTURE(Start_fix, BigNumStep) { // Переполнение при возведении в квадрат
        uint64_t* numbers {new uint64_t[4]{1, 2, 3, 184467440737}};
        CHECK_EQUAL(std::numeric_limits<uint64_t>::max(),p->count(4, numbers));
    }
    TEST_FIXTURE(Start_fix, BigNumPlus) { // Переполнение при сложении
        uint64_t* numbers {new uint64_t[4]{1, 2, 3038000000, 3038000000}};
        CHECK_EQUAL(std::numeric_limits<uint64_t>::max(),p->count(4, numbers));
    }
}


int main(int argc, char **argv)
{
    return UnitTest::RunAllTests();
}
