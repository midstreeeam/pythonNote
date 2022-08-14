'''
An simplified implementation of built-in property class
'''

class Property:
    
    def __init__(self, getter, setter=None):
        self.__getter = getter
        self.__setter = setter
        self.__name__ = getter.__name__


    def __get__(self, instance, owner=None):
        if instance is None:
            return self
        return self.__getter(instance)


    def __set__(self, instance, value):
        if self.__setter is None:
            raise AttributeError("'{0}' is read-only".format(
                                 self.__name__))
        return self.__setter(instance, value)


    def setter(self, setter):
        self.__setter = setter
        return self
