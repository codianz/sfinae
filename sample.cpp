#include <iostream>
#include <type_traits>

struct A {
    void funcA() {
        std::cout << "funcA called" << std::endl;
    }
};

struct B {
    void funcB() {
        std::cout << "funcB called" << std::endl;
    }
};


struct C {
    void funcC() {
        std::cout << "funcC called" << std::endl;
    }
};


// class T に funcA が存在するかを確認する
template <typename T> class has_funcA {
private:
    // 「int = (&X::funcA, 0)」が謎めいているが順を追って考えると
    //  「&X::funcA」とあるが、こいつはX::funcAの関数へのポインタ（実アドレス）
    // つまり、この時点で X に funcA が存在しないとエラーとなり
    // オーバーロードしている test() 関数の候補から外れる。
    // 更に「(&X::funcA, 0)」の表現であるが、これはコンマ演算子であり、
    // 左から順に評価していって、一番右側の値が採用される。
    // https://ja.wikipedia.org/wiki/%E3%82%B3%E3%83%B3%E3%83%9E%E6%BC%94%E7%AE%97%E5%AD%90
    // すなわち、「&X::funcA」を評価（関数の実アドレスという即値）を行った後、
    // 「0」が採用されるのだけど、代入先が無い！
    // んだけど、仮引数が無くてもデフォルトパラメータは指定できるという気持ち悪い仕様なので
    // 「int value = (&X::funcA, 0)」 ではなく 「int = (&X::funcA, 0)」 としている。
    // まぁ、未使用変数的な感じだし、アリといえばアリだが、なんか気持ち悪いね。
    // （普通の関数でも void foo(int=0) {} は気持ち悪いが問題ないコード）
    // ちなみに、test() 関数は実体が定義されていないが、これは後述する decltype で
    // 返却される型のみを渡すために存在するもので、実行されることはない。
    // （仮に実行されるのであれば、リンク時にエラーとなるけど、実行されることがないのでエラーにならない）
    template <typename X, int = (&X::funcA, 0)>
        static std::true_type test(X*);
    static std::false_type test(...);
public:
    // private では test(X*) と test(...) が定義されている。
    // そして、test((T*)nullptr) を実行した場合（decltypeは評価のみで実行されない）
    // T* を引数にした場合、どちらの関数が採用されるかを判定する。
    // さらに、decltype でその採用される関数の返却値を取得し、valueとする。
    // ちなみに、decltype内では、引数を渡しているが、呼び出しは行われない。
    // つまり、オーバーロードされている関数を選択させるために値を入れるだけで実行されない。
    static constexpr bool value = decltype(test((T*)nullptr))::value;
};


// enable_if_t を使用して、オーバーロードされた call() 関数のうち、どれを採用するかを制御する。
// ただし、enable_if_t は C++14以降の実装

// class T に funcA が存在する場合に呼び出される call()
// ちなみに、テンプレート引数の
//「std::enable_if_t< has_funcA< T>::value, bool> = true」
// こいつが、何やってるのという疑問があると思うが、こいつも、さっきの仮引数の省略（未定義っていうのか？）
// と同じ理屈で
// 「std::enable_if_t< has_funcA< T>::value, bool> bValue = true」
// の bValue を省略している表現。
// んで、本題の採用候補をどうやっているかというと
// enable_if_t の第一テンプレート引数が false の場合、型を返さずエラーとなる
// つまり、オーバーロードされた関数の採用候補から外れるという理屈になる。

template <typename T, std::enable_if_t< has_funcA< T>::value, bool> = true >
    void call(T& x)
{
    std::cout << "funcA exists" << std::endl;
    x.funcA();
}

// class T に funcA が存在しない場合に呼び出される call()
template <typename T, std::enable_if_t< !has_funcA< T>::value, bool> = true >
    void call(T& x)
{
    x.funcB();
    std::cout << "funcA does not exist" << std::endl;
}

int main(void){
    A a;
    B b;
    C c;
    call(a);
    call(b);
    // call(c);  もし、funcA, funcB, funcC を個別に呼びたいなら、個別に判定する hax_xxxx を作成する必要がある。
}
