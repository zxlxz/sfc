#pragma once

#include <ustd/core.h>

namespace ustd::console
{

enum Color: i8
{
    RST = 0,
    BLK = '0',
    RED = '1',
    GRN = '2',
    YEL = '3',
    BLU = '4',
    MAG = '5',
    CYN = '6',
    WHT = '7'
};

// property: columns
fn columns(int fd)->u32;

fn is_term(int fd)          -> bool;
fn set_fg (int fd, Color c) -> bool;
fn set_bg (int fd, Color c) -> bool;

}
