# ImageLabel

## 1.	标注说明
物体的标注可以分成两层，常用的第一层为标注图像上的物体，第二次为在物体上标注细节（比如某一部分的缺陷）。<br/>
Annotation_names.txt 中添加第一层标注物体名称，文件末尾不要留空格，下同。<br/>
Sub_annotation_names.txt 中添加第二层标注物体的名称。<br/>

## 2.	软件界面
按钮<br/>
*选择图像：浏览文件选择单张图像<br/>
*保存图像标注：直接保存在图片所在文件夹，文件名与图片名称相同，后缀.xml<br/>
*关闭系统：<br/>
*选择文件夹：选择一个文件夹，读取文件夹中的jpg和bmp图片，图片显示在下方列表中，并默认打开第一张图片<br/>
*下一张：打开列表框中下一张图片<br/>
*上一张：打开列表框中上一张图片<br/>

## 3.	操作说明
*平移图像：鼠标左键按住非标注区域拖动<br/>
*缩放图像：按住ctrl滑动滚轮<br/>
*标注第一层图像：右键选择 添加物体矩形框，然后绘制矩形框，在跳出对话框中选择<br/>
*标注第二层图像：在第一层物体框内右键选择 添加缺陷，然后绘制矩形框<br/>
*删除物体，删除缺陷：在矩形框内右键选择删除，删除物体时会把该物体对应的所有第二层缺陷全部删除<br/>
*编辑物体：在物体矩形框内右键选择编辑，跳出编辑对话框<br/>
*编辑缺陷：同上<br/>
*移动矩形框：在框内单机鼠标，按住SPACE移动鼠标<br/>
*调整矩形框大小：点击矩形框四个角的蓝色方框，鼠标左键拖动<br/>
