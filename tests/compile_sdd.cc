#include <iostream>
#include <vector>
#include <ctime>
#include <random>

//#define LIBSDD_PACKED
#include "sdd/conf/default_configurations.hh"
#include "sdd/sdd.hh"
#include "sdd/dd/paths.hh"

int
main()
{
  typedef sdd::conf::conf0 conf;
  typedef sdd::dd::SDD<conf> SDD;


//  std::cout << sizeof(SDD) << std::endl;
//  std::cout << sizeof(sdd::dd::SDD<conf>) << std::endl;
//  std::cout << sizeof(sdd::dd::SDD_unique<conf>) << std::endl;
//  std::cout << sizeof(sdd::dd::SDD_data<conf>) << std::endl;
//  std::cout << sizeof(sdd::dd::node<conf, conf::Values>) << std::endl;
//  std::cout << sizeof(sdd::dd::node<conf, sdd::dd::SDD<conf>>) << std::endl;
//  std::cout << std::alignment_of<sdd::dd::node<conf, conf::Values>>::value << std::endl;
//   std::cout << sizeof(sdd::dd::context<conf>::sum_cache_type::cache_entry::operation_) << std::endl;

//   std::size_t nb_paths = sdd::dd::visit(count_path(), x);

  const SDD one(true);

//  {
//    flat_alpha_builder builder;
//    builder.add({1}, SDD(1, {4}, one));
//    builder.add({2}, SDD(1, {4,5}, one));
//    builder.add({3}, SDD(1, {5}, one));
//    ASSERT_EQ( SDD(0, std::move(builder))
//              , sum(cxt, { SDD(0, {1,2}, SDD(1, {4}, one))
//      , SDD(0, {2,3}, SDD(1, {5}, one))}));
//  }

//   SDD x('a', {1,2}, SDD('b', {4}, one));
//   SDD y('a', {2,3}, SDD('b', {5}, one));
//
//   std::cout << x + y << std::endl;

//  flat_alpha_builder builder;
//  builder.add({0,1}, SDD(true));
//  ASSERT_EQ( SDD(0, std::move(builder))
//            , sum(cxt, {SDD(0, {0,1}, one), SDD(0, {1}, one)}));

//  SDD x('a', {0,1}, one);
//  SDD y('a', {1}, one);

//  typedef sdd::dd::flat_node<conf> flat_node;
//  const flat_node& xnode =
//    sdd::internal::mem::variant_cast<flat_node>(x->data());
//  std::cout << xnode.size() << std::endl;

//  std::cout << x << std::endl;
//  std::cout << y << std::endl;
//  std::cout << x + y << std::endl;


//  const SDD zero(false);
//  const SDD y('b', {0}, one);
//  const SDD x('a', {1}, SDD('b', {0}, one));

//  std::cout << "------------" << std::endl;
//
//
//  SDD x('a', {0}, SDD('b', {0}, one));
//  SDD y('a', {0}, SDD('a', {1}, one));
//
//  try
//  {
//    std::cout << x + y << std::endl;
//  }
//  catch (sdd::dd::top<conf>& e)
//  {
//    std::cout << e.what() << std::endl;
//  }
////  std::cout << "x " << x << std::endl;
////  std::cout << "y " << y << std::endl;
//
//  SDD a1('a', {0}, SDD('b', {0}, one));
//  SDD a2('a', {1}, SDD('b', {1}, one));
//  std::cout << "res = " << (a1 + a2) << std::endl;
//  std::cout << count_paths(a1 + a2) << std::endl;

////  x & y;
//
//  std::cout <<  (SDD('a', {0,1}, SDD('b', {0}, one))
//               - SDD('a', {1}, SDD('c', {0}, one)))
//            << std::endl;


//  std::cout << "<<<<<<<<<<<<" << std::endl;
//  x.ptr_.ptr_.x_->decrement_reference_counter();
//  std::cout << "x ref: " << x.ptr_.ptr_.x_->reference_counter() << std::endl;

//  std::cout << sdd::sum({SDD(0, {0}, one), SDD(0, {1}, one)}) << std::endl;
//  std::cout << sdd::intersection({SDD(0, {0,1}, one), SDD(0, {1}, one)}) << std::endl;

//  std::cout << zero.empty() << std::endl;
//  std::cout << zero << std::endl;
//  std::cout << one << std::endl;

//  conf::Values val0;
//  val0.insert(0);

//  SDD x(0, val0, one);
//  std::cout << x << std::endl;

//  SDD y(0, x, SDD(0, val0, one));
//  std::cout << y << std::endl;

//  conf::Values val1;
//  val1.insert(1);
//
//  conf::Values val2;
//  val2.insert(2);
//  val2.insert(3);
//
//  conf::Values val3;
//  val3.insert(3);
//  val3.insert(4);

//  SDD tmp(0, {1}, SDD(0, {1}, one));
//  SDD tmp(0, {1}, one);
//  std::cout << tmp << std::endl;
//  SDD za(0, {1}, SDD(0, {1}, one));
//  std::cout << za << std::endl << std::endl;
//  SDD zb = SDD(0, val2, SDD(1, val2, one));
//  std::cout << zb << std::endl << std::endl;
//  SDD zc = SDD(0, val3, SDD(1, val3, one));
//  std::cout << zc << std::endl << std::endl;
//
//  SDD z0 = zb + zc + za;
//  std::cout << z0 << std::endl << std::endl;
//
//  std::cout << (z0 - zc) << std::endl;
//  std::cout << (za + zb) << std::endl;
//  std::cout << (z0 - zc - za) << std::endl;
//
//  SDD z1 = zc + za + zb;
//  std::cout << z1 << std::endl << std::endl;
//
//  SDD z2 = zb + zc + za;
//  std::cout << z2 << std::endl << std::endl;
//
//  std::cout << std::endl;
//  std::cout << z0 << std::endl << std::endl;
//  std::cout << (z0 == z1) << std::endl << std::endl;
//
//  SDD z = z0 + z1;
//
//
//  std::cout << z << std::endl;
////  z0 += z1;
////  std::cout << z0 << std::endl;
//  z0 += (zc + za + zb);
//  std::cout << z0 << std::endl;


//  {
//    conf::Values val0;
//    conf::Values val1;
//    val0.insert(0);
//    val1.insert(1);
//    std::cout << (node(0, val0, one) + node(0, val1, one)) << std::endl;
//    sdd::dd::global_context<conf>().clear();
//  }
 
//  std::cout << (node(10, x1, one) + node(10, x2, one)) << std::endl;


  const std::size_t size = 20;
  const std::size_t nb_sdd = 100;
  const time_t t0 = time(NULL);
//  try
  {
    for (std::size_t j = 0; j < 5000; ++j)
    {
      std::vector<SDD> vec;
      for (std::size_t k = 0; k < nb_sdd; ++k)
      {
        SDD x = one;
        for (std::size_t i = 0; i < size; ++i)
        {
          x = SDD(static_cast<unsigned char>(i + 63), {static_cast<unsigned int>(random() % 64)}, x);
        }
        vec.push_back(x);
      }
      SDD tmp = sdd::dd::sum<conf>(vec.begin(), vec.end());
      std::cout << count_paths(tmp) << std::endl;
//      std::cout << tmp << std::endl;
    }
  }
//  catch (std::exception& e)
  {
//    std::cout << e.what() << std::endl;
  }
  std::cout << "Time: " << time(NULL) - t0 << "s" << std::endl;

//  conf::Values val0;
//  val0.insert(0);
//  std::cout << node(0, node(1, val0, one), node(1, one, one)) << std::endl;

//  std::cout << "------------" << std::endl;
//  SDD x = node(0, one, one);
//  std::cout << "------------" << std::endl;
//  SDD y = node(1, one, one);
//  std::cout << "------------" << std::endl;
//  std::cout << "x: " << x << " | y: " << y << std::endl;
//
//  using std::swap;
//  swap(x, y);
//  std::cout << "x: " << x << " | y: " << y << std::endl;
  std::cout << "------------" << std::endl;
  return 0;
}
