#include <iostream>
#include <vector>
#include <memory>

class SimpleObject
{
    public:
        int ivar;        
        SimpleObject():ivar(1) {   }
        //Метод
        void foo()
        {
            std::cout<<"call foo method"<<std::endl;
        }
};
//Класс пользовательский удалитель

class Deleter{
    public :

        Deleter()
        {

        }
        auto mydelete()
        {
          std::cout<<"CALL custom deleter"<<std::endl;
		  //Действия перед вызовом деструктора данного объекта
        }
        template<class CD>
        void operator()(CD* p) const
        {
            std::cout << "Call delete for object...\n";
            delete p;
        }
};
//Другой пользовательский удалитель 
class Other_deleter{
    public :
        //Действия перед вызовом деструктора данного объекта
        auto mydelete() // fixme achizhov для чего этот метод?
        {
            std::cout<<"CALL OTHER custom deleter"<<std::endl;
        }
       
        // fixme achizhov почему шаблонный параметр у метода а не у класса
        template<typename CDL_OTHER>
        void operator()(CDL_OTHER* p) const
        {
            std::cout << "Call other delete for object...\n";
            delete p;
        }
};

class FileDeleter{
    public :
        //Действия перед вызовом деструктора данного объекта
        auto mydelete() // fixme achizhov для чего этот метод?
        {
          std::cout<<"CALL custom deleter"<<std::endl;
        }
        // fixme achizhov почему шаблонный параметр у метода а не у класса
        template<typename CDL>
        void operator()(CDL* p) const
        {
            std::cout << "Call delete for object...\n";
            fclose(p);
        }
};

//Класс для реализации альтернативы функционалу std::unique_ptr
template<typename T,typename D=Deleter>
class Intellpointer
{
    public:
        explicit Intellpointer(T *ptr) : pntr(ptr)
        {
        }
        explicit Intellpointer(T *ptr,D dltr) : pntr(ptr),objdel(dltr) 
        {
        }

        //Запрет конструктора копирования
        Intellpointer(const Intellpointer& obj_t) = delete;

		Intellpointer(Intellpointer<T ,D>&& obj_t) noexcept
        {
            T* temp_pointer=pntr;
            pntr=obj_t.get();
            obj_t.reset( temp_pointer ); // fixme achizhov неверно
            std::cout<<"MOVE Constructor "<< &pntr << std::endl;
        }

        ~Intellpointer()
        {
            //this->reset(); // fixme achizhov неверно
            D& dltr= this->get_deleter();
            
        }
        //Методы
        D& get_deleter() noexcept
        {
            return objdel;
        }
        const D& get_deleter() const noexcept
        {
            return objdel;
        }
        
        T* get()  const noexcept
        {
            return pntr; 
        } 
        T* release() noexcept
        {

            pntr=nullptr;

            return pntr;
        }

        // fixme achizhov неверно
        void reset() noexcept
        {
            this->pntr=nullptr;
        }
        // fixme achizhov неверно
        void reset(T* newptr) noexcept
        {
            pntr=newptr;
        }
        void swap(Intellpointer& other) noexcept
        {
            T* temp_pointer=pntr;
            pntr=other.get();
            other.pntr=temp_pointer;
        }
        
        Intellpointer& operator=(const Intellpointer& other) = delete;
        
        
        T* operator->() const noexcept{
            return this->get();
        }
        
        T& operator*() const{
             return *this->get();
        }

        Intellpointer& operator=(Intellpointer&& other) // fixme achizhov неверно
        {
            this->reset(other.pntr);
            other.release();
            std::cout << &pntr << " MOVE assigned \n";
            return *this;
        }

    private:
        T* pntr;

        int count_pointer=0;
        D objdel;
        
        
};
//Специализация класса, для работы с массивами
template <class T1>
class Intellpointer<T1[]> 
{
    public:
    //Конструкторы
    Intellpointer(T1 *ptrs):pntr(ptrs)
    {}
    //Запрет конструктора копирования
    Intellpointer& operator=(const Intellpointer& other) = delete;
    //Деструктор
    ~Intellpointer()
    {
        std::cout<< "desctruct array[]" <<std::endl;
    }
    //Методы
    T1* get()  const noexcept
    {
        return pntr; 
    } 

    T1* release() noexcept
    {
        pntr=nullptr;
        return pntr;
    }
    template <class U>
    void reset(U p) noexcept = delete;
    void reset() noexcept
    {
        //Еще раз для очистки данных по указателю
        pntr=nullptr;
    }
    void reset(T1* newptr) noexcept
    {
        pntr=newptr;
    }
    void swap(Intellpointer& other) noexcept
    {
        std::cout <<  " SWAP[] assigned \n";
        
        T1* temp_pointer=pntr;
        pntr=other.get();
        other.reset( temp_pointer);
    }
    Intellpointer<T1[]>& operator=(Intellpointer<T1[]>&& other)
    {
        this->reset( other.pntr);
        other.release();
        std::cout << &pntr << " MOVE assigned \n";
        return *this;
    }
    private:
    T1* pntr;
};
void f(const SimpleObject& foo) 
{
    std::cout << "тест оператора * \n";
}
int main() 
{
    // fixme achizhov все тесты по отдельным методам

    
    auto mydeleter=[](){
        std::cout<<" test deleter " <<  std::endl;
    };
    //Вызов unique_ptr для сравнения работы
    std::cout<< "  Работа  unique_ptr для сравнения:" << std::endl;    
    
    SimpleObject *so1=new SimpleObject();

    std::unique_ptr<SimpleObject> up1(new SimpleObject);
    std::unique_ptr<SimpleObject> up2(std::move(so1)); //Конструктор перемещения

    std::cout<<"up1= "<< up1.get()<<" up2= "<< up2.get() <<  std::endl;
    up2.swap(up1);
    std::cout<<"up2.swap(up1)"<<  std::endl;
    std::cout<<"up1= "<< up1.get()<<" up2= "<< up2.get() <<  std::endl;
    
    up2.swap(up1);
    std::cout<<"swap up2 <-> up1: \n"<<"up1= "<< up1.get()<<" up2= "<< up2.get() <<  std::endl;
    //std::cout<<"up2.swap(up1)"<<  std::endl;
    SimpleObject *zrelA=up1.release();
    std::cout<<"release up1: \n"<<"up1= "<< up1.get()<<" up2= "<< up2.get() <<  std::endl;
    up1.reset(new SimpleObject);
    std::cout<<"reset up1: \n"<<"up1= "<< up1.get()<<" up2= "<< up2.get() <<  std::endl;
    
    //Перегрузка операторов
    up2->foo();//Оператор ->
    f(*up2); //Оператор 



std::cout<< "  Работа  Intellpointer для сравнения:" << std::endl;   
    SimpleObject *pso1=new SimpleObject();
    SimpleObject *pso2=new SimpleObject();


    Intellpointer<SimpleObject> ip1(pso1);
    Intellpointer<SimpleObject> ip2(new SimpleObject());



    std::cout<<"ip1= "<< ip1.get()<<" ip2= "<< ip2.get() <<  std::endl;
    ip2.swap(ip1);
    std::cout<<"ip2.swap(ip1)"<<  std::endl;
    std::cout<<"ip1= "<< ip1.get()<<" ip2= "<< ip2.get() <<  std::endl;
    
    ip2.swap(ip1);
    std::cout<<"swap ip2 <-> ip1: \n"<<"ip1= "<< ip1.get()<<" ip2= "<< ip2.get() <<  std::endl;
    //std::cout<<"ip2.swap(ip1)"<<  std::endl;
    SimpleObject *zrelB=ip1.release();
    std::cout<<"release ip1: \n"<<"ip1= "<< ip1.get()<<" ip2= "<< ip2.get() <<  std::endl;
    ip1.reset(new SimpleObject);
    std::cout<<"reset ip1: \n"<<"ip1= "<< ip1.get()<<" ip2= "<< ip2.get() <<  std::endl;

    //Перегрузка операторов
    ip2->foo();//Оператор ->
    f(*ip1); //Оператор 



    //Использование пользовательского удалителя в  std::unique_ptr
    std::unique_ptr<SimpleObject, Deleter> up(new SimpleObject(), Deleter());
    Deleter &del = up.get_deleter();
    del.mydelete();

    //Использование пользовательского удалителя в Intellpointer
    Intellpointer<SimpleObject,Deleter> spp4A(new SimpleObject(),Deleter());
    Deleter &del2 = spp4A.get_deleter();
    del2.mydelete();

    Intellpointer<SimpleObject,Other_deleter> spp5B(new SimpleObject(),Other_deleter());
    Other_deleter& del3 = spp5B.get_deleter();
    del3.mydelete();

    
    //Лямбда удалитель
    auto lmbd_deleter=[](SimpleObject *lsmo)
    {
        std::cout<<"Lambda deleter"   << std::endl;
    };
    //std::unique_ptr<SimpleObject, decltype(lmbd_deleter)> test111(new SimpleObject(), lmbd_deleter);
    //Intellpointer<SimpleObject,decltype(lmbd_deleter)> spp6B(new SimpleObject(),lmbd_deleter);


return 0;   
/*
    

    Intellpointer<SimpleObject> spp1A(pso1);
    Intellpointer<SimpleObject> spp2A(new SimpleObject());
  */  
 
  
    std::cout<<"----------------------------------------" << std::endl;

    int *pi2= new int(22);
//Сравниваем работу с unique_ptr
    std::cout<<" Создаем пару объектов класса  Intellpointer pointer1 и pointer2 " << std::endl;       
    Intellpointer<int> myptr1(new int(444));
    Intellpointer<int> myptr2(pi2);

    std::cout<<" pointer1 " << myptr1.get() <<" *pointer2 "<< myptr2.get() << " значение pointer2: =" << *pi2 << std::endl;
    myptr2.reset();

    std::cout<<" pointer1 " << myptr1.get() <<" *pointer2 "<< myptr2.get()  << " значение pointer2:  =" << *pi2  << std::endl;

    std::cout<<"----------------------------------------" << std::endl;
    std::unique_ptr<int[]> pArr1(new int[10]);
    std::unique_ptr<int[]> pArr2(new int[8]);
    
    std::cout<<" Upointer1[] " << pArr1.get() << " : "  <<" Upointer2[] "<< pArr2.get() << std::endl;
    pArr1.swap(pArr2);
    pArr1.swap(pArr2);
    int *zrelD=pArr2.release();
    std::cout<<" Upointer1[] " << pArr1.get() << " : "  <<" Upointer2[] "<< pArr2.get() << "=>"<< *zrelD    << std::endl;

    SimpleObject *psso=new SimpleObject();
    SimpleObject *psso2=new SimpleObject();

    Intellpointer<SimpleObject> spp1A(psso);
    Intellpointer<SimpleObject> spp2A(new SimpleObject());
    std::cout<<" Spointer1 " << spp1A.get() <<" Spointer2 "<< spp2A.get()  << std::endl;

    spp2A.swap(spp1A);
    spp2A->foo();
    f(*spp2A);

    std::cout<<" Spointer1 " << spp1A.get() <<" Spointer2 "<< spp2A.get()   << std::endl;

    std::unique_ptr<SimpleObject> uso(new SimpleObject);
    uso->foo();
    f(*uso);



    return 0;
}