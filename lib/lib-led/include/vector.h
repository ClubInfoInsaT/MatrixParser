///DonArkada (da::)
///Own version of std::vector

#ifndef VECTOR_H
#define VECTOR_H


namespace da
{
    typedef unsigned int size_t;
    template<typename _Tp>
    class vector
    {
///===========PRIVATE OF VECTOR==============================
        private:
            _Tp *v_element;
            _Tp *v_element2;
            size_t v_counter;   //number of elements
            size_t v_capacity;     //actual size of defined array

///===========PUBLIC OF VECTOR===============================
        public:

            ~vector(){delete []v_element;}
            vector():v_counter(1),v_capacity(2)
            {
                v_element = new _Tp[v_capacity];
            }

            vector(size_t n):v_counter(n),v_capacity(n)
            {
                v_element = new _Tp[n];
            }
    ///===============RETURN NUMBER OF ELEMENTS================================
            size_t size()
            {
                return v_counter;
            }
    ///===============RETURN SIZE OF ALLOCATED STORAGE=========================
            size_t capacity()
            {
                return v_capacity;
            }



    ///===============ADD ELEMENT==============================================
            void push_back(const _Tp &el)
            {
                if(v_counter==v_capacity)
                {
                    double *n_size = new double;
                    *n_size=v_capacity*1.5;
                    v_capacity=*n_size;
                    delete n_size;
                    v_element2 = new _Tp[v_capacity];

                    //for(size_t i=0;i<v_counter;i++){v_element2[i]=v_element[i];}
                    memcpy(v_element2,v_element,v_counter*sizeof(_Tp));
                    delete []v_element;
                    v_element=new _Tp[v_capacity];
                    //for(size_t i=0;i<v_counter;i++){v_element[i]=v_element2[i];}
                    memcpy(v_element,v_element2,v_counter*sizeof(_Tp));
                    delete []v_element2;
                }
                v_element[v_counter++]=el;

            }
    ///===============SWAP TWO ELEMENTS========================================
            void swap(size_t v1,size_t v2)
            {
                _Tp *temp=new _Tp;
                *temp=v_element[v1];
                v_element[v1]=v_element[v2];
                v_element[v2]=*temp;
                delete temp;
            }
            ///======================================================================

            _Tp &operator[](size_t n)
            {
                if(n<v_counter)return v_element[n];
                else return v_element[v_counter-1];
            }
            ///======================================================================




    };



}




#endif
