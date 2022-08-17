// My first c extension for py

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#define PyInt_AsLong(x) (PyLong_AsLong((x))) 

// This is a C-extension for Python to calculate Pagerank
// This is just a bold attempt, I can not make sure that there
// is no memery leak. The C code will be revised.

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

#define min(a, b) ((a) > (b) ? (b) : (a))
#define equal(a, b)	((a-b)<1e-7 && (a-b)>-(1e-7))

typedef struct  { int row, col; float **element; }Mat;

void swap(int *a, int *b)
{
	int m;
	m = *a;
	*a = *b;
	*b = m;
}
 
void perm(int list[], int k, int m, int* p, Mat* mat, float* det) 
{
	int i;

	if(k > m){
		float res = mat->element[0][list[0]];

		for(i = 1; i < mat->row ; i++){
			res *= mat->element[i][list[i]];
		}

		if(*p%2){
			//odd is negative
			*det -= res;
		}else{
			//even is positive
			*det += res;
		}
	}
	else{
		// if the element is 0, we don't need to calculate the value for this permutation
		if(!equal(mat->element[k][list[k]], 0.0f))
			perm(list, k + 1, m, p, mat, det);
		for(i = k+1; i <= m; i++)
		{
			if(equal(mat->element[k][list[i]], 0.0f))
				continue;
			swap(&list[k], &list[i]);
			*p += 1;
			perm(list, k + 1, m, p, mat, det);
			swap(&list[k], &list[i]);
			*p -= 1; 
		}
	}
}

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

/* dst = src1 - src2 */
Mat* MatSub(Mat* src1, Mat* src2, Mat* dst)
{
	int row, col;

#ifdef MAT_LEGAL_CHECKING
	if( !(src1->row == src2->row && src2->row == dst->row && src1->col == src2->col && src2->col == dst->col) ){
		printf("err check, unmatch matrix for MatSub\n");
		MatDump(src1);
		MatDump(src2);
		MatDump(dst);
		return NULL;
	}
#endif

	for(row = 0 ; row < src1->row ; row++){
		for(col = 0 ; col < src1->col ; col++){
			dst->element[row][col] = src1->element[row][col] - src2->element[row][col];
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

void MatCopy(Mat* src, Mat* dst)
{
	int row, col;
	
#ifdef MAT_LEGAL_CHECKING
	if( src->row != dst->row || src->col != dst->col){
		printf("err check, unmathed matrix for MatCopy\n");
		MatDump(src);
		MatDump(dst);
		return ;
	}
#endif
	
	for(row = 0 ; row < src->row ; row++){
		for(col = 0 ; col < src->col ; col++)
			dst->element[row][col] = src->element[row][col];
	}
}

Mat* MatEnlarge(Mat* src, Mat* dst, float alpha){

	for(int i=0;i<src->row;i++){
		for(int j=0;j<src->col;j++){
			dst->element[i][j]=src->element[i][j]*alpha;
		}
	}
	return dst;
}



Mat trans_t, temp, trans_ret, rank, inipr, inimat, t, x, y, pagerank, pre_matrix;


Mat mat_tran(Mat *mat){

	//init
	int len=mat->col;
	// Mat trans_t;
	MatCreate(&trans_t,len,len);
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

	MatCreate(&temp,len,len);
	MatCreate(&trans_ret,len,len);
	MatSetVal(&temp,trans_matrix);
	MatTrans(&temp,&trans_ret);

    return trans_ret;
}


Mat pr(float **a,int len, float alpha, int max_iter){

	// init
	// float f_rank[len];
	float f_inipr[len];
	for(int i=0;i<len;i++){
		f_inipr[i]=1/(float)len;
	}
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
	// MatDump(&rank);

	return rank;
}


PyListObject * get_pagerank(float ** arr, int len){
	int max_iter = 100;
    //int len = 4;
	float alpha = 0.8;
    // float arr[4][4]={ 
	// {0.0f, 1.0f, 0.0f, 0.0f}, 
	// {1.0f, 0.0f, 0.0f, 1.0f}, 
	// {1.0f, 1.0f, 0.0f, 0.0f},
	// {1.0f, 0.0f, 1.0f, 0.0f}};
    pagerank = pr(arr,len,alpha,max_iter);
	// MatDump(&pagerank);


	// free memery space
	MatDelete(&trans_ret);
	MatDelete(&trans_t);
	MatDelete(&temp);
	// MatDelete(&rank); // MatDelete(&y); MatDelete(&pagerank); same place
	MatDelete(&inimat);
	MatDelete(&inipr);
	MatDelete(&t);
	MatDelete(&x);
	MatDelete(&pre_matrix);

    PyListObject *list;
    list = (PyListObject *) Py_BuildValue("[]");
    for(int i=0;i<pagerank.row;i++){
        PyObject* pyfloat = (PyObject *)Py_BuildValue("f",pagerank.element[i][0]);
        PyList_Append(list,pyfloat);
    }

    MatDelete(&pagerank);

	return list;
}


static PyObject* ssoPr_pr(PyObject* self, PyObject* args) 
{
    PyObject* adjMatrix; //get matrix in python list
   
    if (! PyArg_ParseTuple( args, "O", &adjMatrix))
        return NULL;

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

    return get_pagerank(c_adjMatrix,length);
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