
#ifndef RECORDTYPE_H
#define RECORDTYPE_H

#include <vector>
#include <sstream>

class ValueRecord
{
public:
  std::vector<double> values;
  void add( double newValue ) { values.push_back( newValue ); };
  void set( int index, double newValue ){ values[index] = newValue; };
  void increment( int index ){ values[index]++; };
  double get( int index ) { return values[index]; };
  int size(){ return values.size(); };
};
  

class LabelRecord : public ValueRecord
{
public:
  int label;
  void setLabel( int newLabel ) { label = newLabel; };
  int getLabel() { return label; };
};


class TimeLabelRecord : public LabelRecord
{
public:
  double time;
  void setTime( double newTime ) { time = newTime; };
  double getTime() { return time; };
};


class MarkovRecord
{
public:
  int state;
  int symbol;
  void setState( int newState ) { state = newState; };
  void setSymbol( int newSymbol ) { symbol = newSymbol; };
  int getState() { return state; };
  int getSymbol() { return symbol; };
};


#endif