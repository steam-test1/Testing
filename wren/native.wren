// Unfortunately, we have to import the entirety of the old native module in to keep backwards compatibility
// Since we have to remove dynamic_import from C++ and move it to Wren (since an updated version of Wren is
//  less forgiving about the reentrant calls it always said weren't supported) we have to change the source
//  file, which I somewhat stupidly put in the base mod. We don't want a user's game to crash if they update
//  either the basemod or the plugin but not both at the exact same time, so we have to hack around it by
//  overriding the file and implementing dynamic_import in Wren.
//
// This line binds it to the correct module name:
// meta:module=base/native

import "meta" for Meta

class Logger {
	foreign static log(text)
}

class IO {
	foreign static listDirectory(path, dirs)
	foreign static info(path) // returns: none, file, dir
	foreign static read(path) // get file contents
	foreign static idstring_hash(data) // hash a string
	foreign static load_plugin(filename) // load an external plugin
	foreign static has_native_module(name) // returns true if the given module path represents a embedded-in-DLL module

	static dynamic_import(path) {
        // import a file dynamically
        Meta.eval("import \"%(path)\"")
	}
}

foreign class XML {
	construct new(text) {}
	foreign static try_parse(text) // Basically a fancy constructor

	foreign type
	foreign text
	foreign text=(val)
	foreign string
	foreign name
	foreign name=(val)
	foreign [name] // attribute
	foreign [name]=(val) // attribute
	foreign attribute_names

	foreign create_element(name)
	foreign delete()

	foreign detach()
	foreign clone()
	foreign attach(child)
	foreign attach(child, prev_child)

	// Structure accessors
	foreign next
	foreign prev
	foreign parent
	foreign first_child
	foreign last_child

	// Helpers
	is_element {
		return !this.name.startsWith("!--")
	}

	next_element {
		var elem = next
		while(elem != null) {
			if(elem.is_element) break
			elem = elem.next
		}
		return elem
	}

	ensure_element_next {
		var elem = this
		while(elem != null) {
			if(elem.is_element) break
			elem = elem.next
		}
		return elem
	}

	element_children {
		var arr = []
		var elem = first_child
		while(elem != null) {
			if(elem.is_element) arr.add(elem)
			elem = elem.next
		}
		return arr
	}
}
