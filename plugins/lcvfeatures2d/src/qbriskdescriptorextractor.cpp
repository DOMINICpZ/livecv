/****************************************************************************
**
** Copyright (C) 2014-2018 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#include "qbriskdescriptorextractor.h"
#include "opencv2/features2d.hpp"

QBriskDescriptorExtractor::QBriskDescriptorExtractor(QQuickItem *parent) :
    QDescriptorExtractor(cv::BRISK::create(), parent){
}

QBriskDescriptorExtractor::~QBriskDescriptorExtractor(){
}

void QBriskDescriptorExtractor::initialize(const QVariantMap &){
    initializeExtractor(cv::BRISK::create());
}
