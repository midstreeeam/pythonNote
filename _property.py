
# An simplified implementation of built-in property class

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



# can find that class property inherit from class DynamicClassAttribute

class DynamicClassAttribute:
    """Route attribute access on a class to __getattr__.

    This is a descriptor, used to define attributes that act differently when
    accessed through an instance and through a class.  Instance access remains
    normal, but access to an attribute through a class will be routed to the
    class's __getattr__ method; this is done by raising AttributeError.

    This allows one to have properties active on an instance, and have virtual
    attributes on the class with the same name.  (Enum used this between Python
    versions 3.4 - 3.9 .)

    Subclass from this to use a different method of accessing virtual attributes
    and still be treated properly by the inspect module. (Enum uses this since
    Python 3.10 .)

    """
    def __init__(self, fget=None, fset=None, fdel=None, doc=None):
        self.fget = fget
        self.fset = fset
        self.fdel = fdel
        # next two lines make DynamicClassAttribute act the same as property
        self.__doc__ = doc or fget.__doc__
        self.overwrite_doc = doc is None
        # support for abstract methods
        self.__isabstractmethod__ = bool(getattr(fget, '__isabstractmethod__', False))

    def __get__(self, instance, ownerclass=None):
        if instance is None:
            if self.__isabstractmethod__:
                return self
            raise AttributeError()
        elif self.fget is None:
            raise AttributeError("unreadable attribute")
        return self.fget(instance)

    def __set__(self, instance, value):
        if self.fset is None:
            raise AttributeError("can't set attribute")
        self.fset(instance, value)

    def __delete__(self, instance):
        if self.fdel is None:
            raise AttributeError("can't delete attribute")
        self.fdel(instance)

    def getter(self, fget):
        fdoc = fget.__doc__ if self.overwrite_doc else None
        result = type(self)(fget, self.fset, self.fdel, fdoc or self.__doc__)
        result.overwrite_doc = self.overwrite_doc
        return result

    def setter(self, fset): 
        # midstream comment: basically the same as func setter in line 26.
        # (but seems it returns a new class object? why?)
        result = type(self)(self.fget, fset, self.fdel, self.__doc__) # a strange way to build a new class object.
        result.overwrite_doc = self.overwrite_doc 
        return result

    def deleter(self, fdel):
        result = type(self)(self.fget, self.fset, fdel, self.__doc__)
        result.overwrite_doc = self.overwrite_doc
        return result




# class property in Cpython source code

class property(DynamicClassAttribute):
    """
    This is a descriptor, used to define attributes that act differently
    when accessed through an enum member and through an enum class.
    Instance access is the same as property(), but access to an attribute
    through the enum class will instead look in the class' _member_map_ for
    a corresponding enum member.
    """

    member = None

    def __get__(self, instance, ownerclass=None):
        if instance is None:
            if self.member is not None:
                return self.member
            else:
                raise AttributeError(
                        '%r has no attribute %r' % (ownerclass, self.name)
                        )
        else:
            if self.fget is None:
                if self.member is None:   # not sure this can happen, but just in case
                    raise AttributeError(
                            '%r has no attribute %r' % (ownerclass, self.name)
                            )
                # issue warning deprecating this behavior
                import warnings
                warnings.warn(
                        "`member.member` access (e.g. `Color.RED.BLUE`) is "
                        "deprecated and will be removed in 3.14.",
                        DeprecationWarning,
                        stacklevel=2,
                        )
                return self.member
                # XXX: uncomment in 3.14 and remove warning above
                # raise AttributeError(
                #         '%r member has no attribute %r' % (ownerclass, self.name)
                #         )
            else:
                return self.fget(instance)

    def __set__(self, instance, value):
        if self.fset is None:
            raise AttributeError(
                    "<enum %r> cannot set attribute %r" % (self.clsname, self.name)
                    )
        else:
            return self.fset(instance, value)

    def __delete__(self, instance):
        if self.fdel is None:
            raise AttributeError(
                    "<enum %r> cannot delete attribute %r" % (self.clsname, self.name)
                    )
        else:
            return self.fdel(instance)

    def __set_name__(self, ownerclass, name):
        self.name = name
        self.clsname = ownerclass.__name__

# Seems function in source code does not do more things than the simplified version?



# test here
# class C():
#     p=0
    
#     @property
#     def pp(self):
#         return self.p
    
#     @pp.setter
#     def pp(self,value):
#         self.p=value


# c=C()
# print(c.pp)