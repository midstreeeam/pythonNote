// cpython extension to realize pagerank


#define PY_SSIZE_T_CLEAN
#include "Python.h"

#define PyInt_AsLong(x) (PyLong_AsLong((x))) 

// This is a C-extension for Python to calculate Pagerank
// This is just a bold attempt, I can not make sure that there
// is no memery leak. The C code will be revised.

// Usage
// Need to be built using distutils.core before import.
// Take an python list as input and return a list of pagerank.

// Example
// >>> import ssoPr
// >>> mat=[]
// >>> mat.append([0,1,0,0])
// >>> mat.append([1,0,0,1])
// >>> mat.append([1,1,0,0])
// >>> mat.append([1,0,1,0])
// >>> ssoPr.pr(mat)
// [0.17778392136096954, 0.31945979595184326, 0.22519296407699585, 0.2775634229183197]

#define MAT_LEGAL_CHECKING

typedef struct  { int row, col; float **element; } Mat;

Mat* MatCreate(Mat* mat, int row, int col)
{
	int i;

	mat->element = (float**)malloc(row * sizeof(float*));
	if(mat->element == NULL){
		printf("mat create fail!\n");
		return NULL;
	}
	for(i = 0 ; i < row ; i++){
		mat->element[i] = (float*)malloc(col * sizeof(float));	
		if(mat->element[i] == NULL){
			int j;
			printf("mat create fail!\n");
			for(j = 0 ; j < i ; j++)
				free(mat->element[j]);
			free(mat->element);
			return NULL;
		}
	}

	mat->row = row;
	mat->col = col;

	return mat;
}

void MatDelete(Mat* mat)
{
	if(mat->element!=NULL){
		for(int i = 0 ; i<mat->row ; i++){
			if(mat->element[i]!=NULL){
				free(mat->element[i]);
				mat->element[i]=NULL;
			}
		}
	}
	
	if(mat->element!=NULL){
		free(mat->element);
		mat->element=NULL;
	}
}

Mat* MatSetVal(Mat* mat, float* val)
{
	int row,col;

	for(row = 0 ; row < mat->row ; row++){
		for(col = 0 ; col < mat->col ; col++){
			mat->element[row][col] = val[col + row * mat->col];
		}
	}
	return mat;
}

void MatDump(const Mat* mat)
{
	int row,col;

#ifdef MAT_LEGAL_CHECKING
	if(mat == NULL){
		return ;
	}
#endif

	printf("Mat %dx%d:\n", mat->row, mat->col);
	for(row = 0 ; row < mat->row ; row++){
		for(col = 0 ; col < mat->col ; col++){
			printf("%.4f\t", mat->element[row][col]);
		}
		printf("\n");
	}
}

Mat* MatZeros(Mat* mat)
{
	int row,col;

	for(row = 0 ; row < mat->row ; row++){
		for(col = 0 ; col < mat->col ; col++){
			mat->element[row][col] = 0.0f;
		}
	}

	return mat;
}

/* dst = src1 + src2 */
Mat* MatAdd(Mat* src1, Mat* src2, Mat* dst)
{
	int row, col;

#ifdef MAT_LEGAL_CHECKING
	if( !(src1->row == src2->row && src2->row == dst->row && src1->col == src2->col && src2->col == dst->col) ){
		printf("err check, unmatch matrix for MatAdd\n");
		MatDump(src1);
		MatDump(src2);
		MatDump(dst);
		return NULL;
	}
#endif

	for(row = 0 ; row < src1->row ; row++){
		for(col = 0 ; col < src1->col ; col++){
			dst->element[row][col] = src1->element[row][col] + src2->element[row][col];
		}
	}

	return dst;
}

/* dst = src1 * src2 */
Mat* MatMul(Mat* src1, Mat* src2, Mat* dst)
{
	int row, col;
	int i;
	float temp;

#ifdef MAT_LEGAL_CHECKING
	if( src1->col != src2->row || src1->row != dst->row || src2->col != dst->col ){
		printf("err check, unmatch matrix for MatMul\n");
		MatDump(src1);
		MatDump(src2);
		MatDump(dst);
		return NULL;
	}
#endif

	for(row = 0 ; row < dst->row ; row++){
		for(col = 0 ; col < dst->col ; col++){
			temp = 0.0f;
			for(i = 0 ; i < src1->col ; i++){
				temp += src1->element[row][i] * src2->element[i][col];
			}
			dst->element[row][col] = temp;
		}
	}

	return dst;
}

/* dst = src' */
Mat* MatTrans(Mat* src, Mat* dst)
{
	int row, col;

#ifdef MAT_LEGAL_CHECKING
	if( src->row != dst->col || src->col != dst->row ){
		printf("err check, unmatch matrix for MatTranspose\n");
		MatDump(src);
		MatDump(dst);
		return NULL;
	}
#endif

	for(row = 0 ; row < dst->row ; row++){
		for(col = 0 ; col < dst->col ; col++){
			dst->element[row][col] = src->element[col][row];
		}
	}

	return dst;
}

Mat* MatEnlarge(Mat* src, Mat* dst, float alpha){

	for(int i=0;i<src->row;i++){
		for(int j=0;j<src->col;j++){
			dst->element[i][j]=src->element[i][j]*alpha;
		}
	}
	return dst;
}


Mat mat_tran(Mat *mat){

	//init
	int len=mat->col;
	
	Mat trans_t, temp, trans_ret;

	MatCreate(&trans_t,len,len);
	MatCreate(&temp,len,len);
	MatCreate(&trans_ret,len,len);


	MatTrans(mat,&trans_t);
	float **adj_matrix=trans_t.element;
	//MatDump(&t);

	float trans_matrix[len][len];
    int count[len];//count how many links from index i pointing to other pages.
    memset(trans_matrix,0,sizeof(trans_matrix));
    memset(count,0,sizeof(count));
    
    // arr[i][j] means link from i point to j
    int i,j;
    // counting how many links each column
    for(i = 0; i < len; i++){
		for(j = 0;j < len; j++){
		    //float ele = *((float*)adj_matrix+i*len+j); //value at adj_matrix[i][j]
			float ele = trans_t.element[i][j];
			//printf("%f\t",ele);
            if(ele == 1.0f){
                count[i]++;
            }
        }
	}
	// assign value to trans_matrix
    for(i=0;i<len;i++){
        for(j=0;j<len;j++){
            float ele = trans_t.element[i][j];
            if(ele == 1.0f){
                //printf("%f\n",1/(float)count[i]);
                trans_matrix[i][j]=1/(float)count[i];
            }else{
                trans_matrix[i][j]=0.0f; // just in case
            }
        }
    }

	MatSetVal(&temp,trans_matrix);
	MatTrans(&temp,&trans_ret);

	MatDelete(&trans_t);
	MatDelete(&temp);

    return trans_ret;
}


Mat pr(float **a,int len, float alpha, int max_iter){

	// init
	// float f_rank[len];
	float f_inipr[len];
	float avg_pr=1/(float)len;
	for(int i=0;i<len;i++){
		f_inipr[i]=avg_pr;
	}

	Mat rank,inipr,inimat,t,x,y,pre_matrix;

	MatCreate(&rank,len,1);
	MatCreate(&inipr,len,1);
	MatCreate(&inimat,len,len);
	MatCreate(&t,len,1);
	MatCreate(&x,len,1);
	MatCreate(&y,len,1);

	MatSetVal(&rank,f_inipr);
	MatSetVal(&inipr,f_inipr);
	MatSetVal(&inimat,a);

	//calcualtion
	//MatDump(&inimat);
	pre_matrix = mat_tran(&inimat);
	//MatDump(&pre_matrix);
	//MatDump(&inipr);
	
	for(int i=0;i<max_iter;i++){
		MatMul(&pre_matrix, &rank, &t);
		MatEnlarge(&t, &t, alpha);
		MatEnlarge(&inipr, &x, 1.0f-alpha);
		MatAdd(&t,&x,&y);
		rank=y;
	}
	
	// free memery space
	MatDelete(&inimat);
	MatDelete(&inipr);
	MatDelete(&pre_matrix);
	MatDelete(&t);
	MatDelete(&x);

	return rank;
}


PyListObject * get_pagerank(float ** arr, int len, float alpha, int max_iter){

    Mat pagerank = pr(arr,len,alpha,max_iter);
	// MatDump(&pagerank);

    PyListObject *list;
    list = (PyListObject *) Py_BuildValue("[]");
    for(int i=0;i<pagerank.row;i++){
        PyObject* pyfloat = (PyObject *)Py_BuildValue("f",pagerank.element[i][0]);
        PyList_Append((PyObject*)list,pyfloat);
    }

    MatDelete(&pagerank);

	return list;
}


static PyObject* ssoPr_pr(PyObject* self, PyObject* args) 
{
	PyObject* arglist;
	PyObject* arg1; // alpha
	PyObject* arg2; // max_iter
    PyObject* adjMatrix; //get matrix in python list
   
    if (! PyArg_ParseTuple( args, "O", &arglist))
        return NULL;
	
	// get args
	adjMatrix=PyList_GetItem(arglist,0);
	arg1=PyList_GetItem(arglist,1);
	arg2=PyList_GetItem(arglist,2);
	float alpha = (float)PyFloat_AsDouble(arg1);
	int max_iter = (int)PyInt_AsLong(arg2);


    long length = PyList_Size(adjMatrix); // matrix should be square

    float c_adjMatrix[length][length];

    for (int i = 0; i < length; i++)
    {
        PyObject* tempList = PyList_GetItem(adjMatrix, i); // tempList is a column of adjMatrix
        for (int j = 0; j < length; j++){
            PyObject* tempValue = PyList_GetItem(tempList, j);
            long elem = PyInt_AsLong(tempValue); // transfer PyInt to long
            c_adjMatrix[i][j]=(float)elem;
        }
    }

    return get_pagerank(c_adjMatrix,length,alpha,max_iter);
}


static PyMethodDef ssoPr_funcs[] = 
{
    {"pr", (PyCFunction)ssoPr_pr, METH_VARARGS, "Get pr from the given adjacency matrix."},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef ssoPr_module = {
    PyModuleDef_HEAD_INIT,
    "ssoPr",   /* module name */
    "",          /* module doc */
    -1,          /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    ssoPr_funcs
};

PyMODINIT_FUNC PyInit_ssoPr(void)
{
    return PyModule_Create(&ssoPr_module);
}



// py part
// import ssoPr

// def Singleton(cls):
// 	_instance={}
// 	def _singleton(*args,**kwagrs):
// 		if cls not in  _instance:
// 			_instance[cls]=cls(*args,**kwagrs)
// 		return _instance[cls]
// 	return _singleton


// @Singleton
// class PR():
//     '''
//     It is a singleton class. You should always have only one instance of this class.
//     Example:
//     matrix=[[0,1,0,0],
//             [1,0,0,1],
//             [1,1,0,0],
//             [1,0,1,0]]
//     p=PR(matrix)
//     print(p.get_page_rank())
//     >>>[0.17778392136096954, 0.31945979595184326, 0.22519296407699585, 0.2775634229183197]
//     '''
        
//     def __init__(self,mat:list,alpha=0.8,max_iter=100) -> None:
//         if self._mat_checker(mat):
//             self._mat=mat
//         self.alpha=alpha
//         self.max_iter=max_iter
//         pass
    
//     @property
//     def mat(self):
//         return self._mat
    
//     @mat.setter
//     def mat(self,value):
//         if self._mat_checker(value):
//             self._mat=value
    
//     def _mat_checker(self,mat):
        
//         # check instance
//         if not isinstance(mat,list):
//             raise ValueError('input should be pylist')
//         else:
//             for i in mat:
//                 if not isinstance(i,list):
//                     raise ValueError('Input should be matrix in pylist')
        
//         # check shape
//         for i in mat:
//             if len(mat)!=len(i):
//                 raise ValueError('Input should be square matrix')
                
//         # check value  
//         for i in mat:      
//             for j in i:
//                 if j!=0 and j!=1:
//                     raise ValueError('Value in adjency matrix can only be 0 or 1')
//         return True
    

//     def get_page_rank(self):
//         pr_list=ssoPr.pr([self.mat,float(self.alpha),int(self.max_iter)])
//         return pr_list
