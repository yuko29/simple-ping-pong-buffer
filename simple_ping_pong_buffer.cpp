/*****************************************************************************

  Licensed to Accellera Systems Initiative Inc. (Accellera) under one or
  more contributor license agreements.  See the NOTICE file distributed
  with this work for additional information regarding copyright ownership.
  Accellera licenses this file to you under the Apache License, Version 2.0
  (the "License"); you may not use this file except in compliance with the
  License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
  implied.  See the License for the specific language governing
  permissions and limitations under the License.

 *****************************************************************************/

/*****************************************************************************

  simple_fifo.cpp -- Simple SystemC 2.0 producer/consumer example.

                     From "An Introduction to System Level Modeling in
                     SystemC 2.0". By Stuart Swan, Cadence Design Systems.
                     Available at www.accellera.org

  Original Author: Stuart Swan, Cadence Design Systems, 2001-06-18

 *****************************************************************************/

/*****************************************************************************

  MODIFICATION LOG - modifiers, enter your name, affiliation, date and
  changes you are making here.

      Name, Affiliation, Date:
  Description of Modification:

 *****************************************************************************/


#include <systemc.h>

class write_if : virtual public sc_interface
{
   public:
     virtual void write(char) = 0;
     //virtual void reset() = 0;
};

class read_if : virtual public sc_interface
{
   public:
     virtual void read(char &) = 0;
     //virtual int num_available() = 0;
};

class fifo : public sc_channel, public write_if, public read_if
{
   public:
     fifo(sc_module_name name) : sc_channel(name), read_idx(0), write_idx(0), buffer_ptr(0), read_buffer_num(0), write_buffer_num(0) {
       end = false;
       to_read_buffer = buffer0;
       to_write_buffer = buffer1;
     }

     void write(char c) {

       // write operation
       if(c == EOF)
       {
         end = true;
       }
       else
       {

        to_write_buffer[write_idx] = c;
        ++ write_idx;
        ++ write_buffer_num;
        cout << sc_time_stamp() <<  " w buffer" << !buffer_ptr << ": " << c << endl;

        
        // if to_write_buffer is filled 
        // -> keep waiting until to_read_buffer is consumed and the buffers are swapped
        if(write_idx == max)
        {
          write_idx = 0;
          swap_event.notify();
          wait(swap_event);
        }
        
       }
     }

     void read(char &c){
       // if to_read_buffer has been consumed -> swap the two buffer
       if(read_buffer_num == 0)
       {
         if(!end)
         wait(swap_event);
         swap_buffer();
         read_idx = 0;
         swap_event.notify();
       }

       // read operation
       c = to_read_buffer[read_idx];
       ++ read_idx;
       -- read_buffer_num;
       cout << "\t\t" << sc_time_stamp() <<  " r buffer" << buffer_ptr << ": " << c << endl;
       
       if(write_buffer_num == 0 && read_buffer_num == 0) end = false;
     }

     void swap_buffer()
     {
       if(buffer_ptr == 0)
        {
          buffer_ptr = 1;
          to_read_buffer = buffer1;
          to_write_buffer = buffer0;
        }
        else
        {
          buffer_ptr = 0;
          to_read_buffer = buffer0;
          to_write_buffer = buffer1;
        }
      
      write_idx = 0;
      read_buffer_num = write_buffer_num;
      write_buffer_num = 0;
        
     }

     //void reset() { write_idx = 0; }

     //int num_available() { return num_elements;}

   private:
     enum e { max = 10 };
     char buffer0[max];
     char buffer1[max];
     char* to_read_buffer;
     char* to_write_buffer;
     int read_idx, write_idx, buffer_ptr;
     int read_buffer_num, write_buffer_num;
     bool end;
     sc_event write_event, read_event, swap_event;
};

class producer : public sc_module
{
   public:
     sc_port<write_if> out;

     SC_HAS_PROCESS(producer);

     producer(sc_module_name name) : sc_module(name)
     {
       SC_THREAD(main);
     }

     void main()
     {
       const char *str =
         "Visit www.accellera.org and see what SystemC can do for you today!\n";

       while (*str)
       {
         out->write(*str++);
         wait(1, SC_SEC);
       }
         out->write(EOF);

     }
};

class consumer : public sc_module
{
   public:
     sc_port<read_if> in;

     SC_HAS_PROCESS(consumer);

     consumer(sc_module_name name) : sc_module(name)
     {
       SC_THREAD(main);
     }

     void main()
     {
       char c;
       
       while (true) {
         in->read(c);
         wait(1, SC_SEC);
       }
     }
};

class top : public sc_module
{
   public:
     fifo *fifo_inst;
     producer *prod_inst;
     consumer *cons_inst;

     top(sc_module_name name) : sc_module(name)
     {
       fifo_inst = new fifo("Fifo1");

       prod_inst = new producer("Producer1");
       prod_inst->out(*fifo_inst);

       cons_inst = new consumer("Consumer1");
       cons_inst->in(*fifo_inst);
     }
};

int sc_main (int, char *[]) {
   top top1("Top1");
   sc_start(100, SC_SEC);
   return 0;
}
