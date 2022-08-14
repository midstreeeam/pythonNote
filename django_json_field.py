import json
from django.db import models
import numpy as np


'''
For Django Fields

# from db to python
def to_python(self, value):
        if not value:
            return None
        return time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(value))


# call to_python()
def from_db_value(self, value, expression, connection):
        return self.to_python(value)

# from python to db
def get_prep_value(self, value):
        if not value:
            return None
        if isinstance(value, int) or isinstance(value, float):
            return value
        return parser.parse(value).timestamp()

'''


class ListStrField(models.JSONField):
    '''
    Only can store data in [str,str,str] format.
    '''

    def _lst_check(self,value):
        if not isinstance(value,list):
            try:
                raise ValueError('Value {} is not a Python List'.format(str(value)))
            except: raise ValueError('Value should be a Python List')
        for i in value:
            if not isinstance(i,str):
                try:
                    raise ValueError('Value {} is not a Python str'.format(str(value)))
                except: raise ValueError('element in list in should be a Python str')
        return True

    def get_prep_value(self, value):
        self._lst_check(value)
        if value is None:
            return value
        return json.dumps(value, cls=self.encoder)

class NdListStrField(models.JSONField):
    '''
    Can only store nd-list with dtype==str
    '''

    '''
    shape takes input of a tuple, 
    format is exactly the same as numpy.

    When passing the list there should be a wrapper.
    eg. [[
            ['1','1','1'],
            ['1','1','1']
        ]].shape = (1,2,3)
    '''
    
    def __init__(
        self, shape, verbose_name=None, name=None, encoder=None, decoder=None,
        **kwargs,
    ):
        self.shape=shape
        self.arr1d=[]
        super().__init__(verbose_name, name, encoder, decoder,**kwargs)
    
    def _loop_multi_array(self,list_string):
        for list_value in list_string:
            if isinstance(list_value, list):
                self._loop_multi_array(list_value)
            else:
                self.arr1d.append(list_value)
    
    def _lst_check(self,value):
        if not isinstance(value,list):
            val_str=str(value)
            raise ValueError('Value {} is not a Python List'.format(val_str))
        np_value=np.array(value)
        if np_value.shape != self.shape:
            raise ValueError('Shape {} is not a {}'.format(str(np_value.shape),str(self.shape)))
        for i in self.arr1d:
            if not isinstance(i,str):
                val_str=str(i)
                raise ValueError('Value {} is not a Python str'.format(val_str))
        return True

    def get_prep_value(self, value):
        self._loop_multi_array(value)
        self._lst_check(value)
        if value is None:
            return value
        return json.dumps(value, cls=self.encoder)