/*
 * p_obsv.hpp
 *
 *  Created on: Feb 19, 2017
 *      Author: root
 */

#ifndef P_OBSV_HPP_
#define P_OBSV_HPP_

#include "token.hpp"

class p_obsv
{
public:
	virtual void done(bool good)=0;
	virtual void start(std::string prod)=0;
	virtual void end()=0;
	virtual void match(token t)=0;
	virtual ~p_obsv(){};
};


#endif /* P_OBSV_HPP_ */
