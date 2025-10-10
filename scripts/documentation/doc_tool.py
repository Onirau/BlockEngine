import argparse
import re
import json
import sys
from pathlib import Path
from typing import Dict, List, Optional
from dataclasses import dataclass, field

@dataclass
class Parameter:
    name: str
    type: str
    description: str = ""

@dataclass
class Member:
    member_type: str
    name: str
    description: str = ""
    examples: List[str] = field(default_factory=list)
    value_type: str = ""
    default_value: str = ""
    readonly: bool = False
    return_type: str = "void"
    parameters: List[Parameter] = field(default_factory=list)
    deprecated: bool = False
    internal: bool = False
    inherited_from: str = ""

@dataclass
class ClassInfo:
    name: str
    superclass: str = ""
    description: str = ""
    examples: List[str] = field(default_factory=list)
    members: List[Member] = field(default_factory=list)
    source_file: str = ""
    tags: List[str] = field(default_factory=list)

class CommentExtractor:
    # Internal parameter types that should be filtered out
    INTERNAL_PARAM_TYPES = {
        'lua_State',
        'lua_State*',
        'lua_State *',
    }
    
    def __init__(self, src_dir: str, output_file: str):
        self.src_dir = Path(src_dir)
        self.output_file = output_file
        self.classes: Dict[str, ClassInfo] = {}
        self.enums: Dict[str, dict] = {}
        self.datatypes: Dict[str, ClassInfo] = {}
    
    def extract(self):
        print(f"\nScanning directory: {self.src_dir}")
        
        header_files = list(self.src_dir.rglob("*.h"))
        header_files = [f for f in header_files if 'luau' not in str(f) and 'raylib' not in str(f)]
        
        print(f"Found {len(header_files)} header files\n")
        
        for file_path in header_files:
            print(f"Processing: {file_path.name}")
            self.process_file(file_path)
            self.process_enums(file_path)
            # Capture datatypes even without docblocks from src/datatypes
            if 'datatypes' in str(file_path.as_posix()):
                self.process_datatypes(file_path)
        
        print("\nInheriting parent documentation...")
        self.inherit_parent_docs()
        
        api_dump = {
            "Version": 0,
            "Generated": "Auto-Generated",
            "Classes": [],
            "Enums": [],
            "DataTypes": []
        }
        
        for class_info in self.classes.values():
            class_dict = {
                "Name": class_info.name,
                "Superclass": class_info.superclass,
                "Description": class_info.description,
                "Examples": class_info.examples,
                "SourceFile": class_info.source_file,
                "Tags": class_info.tags,
                "Members": []
            }
            
            for member in class_info.members:
                member_dict = {
                    "MemberType": member.member_type,
                    "Name": member.name,
                    "Description": member.description,
                    "Examples": member.examples,
                    "Deprecated": member.deprecated,
                    "Internal": member.internal,
                    "InheritedFrom": member.inherited_from
                }
                
                if member.member_type == "Property":
                    member_dict["ValueType"] = member.value_type
                    member_dict["Default"] = member.default_value
                    member_dict["ReadOnly"] = member.readonly
                elif member.member_type == "Method":
                    member_dict["ReturnType"] = member.return_type
                    member_dict["Parameters"] = [
                        {"Name": p.name, "Type": p.type, "Description": p.description}
                        for p in member.parameters
                    ]
                elif member.member_type == "Event":
                    member_dict["Parameters"] = [
                        {"Name": p.name, "Type": p.type, "Description": p.description}
                        for p in member.parameters
                    ]
                
                class_dict["Members"].append(member_dict)
            
            api_dump["Classes"].append(class_dict)
        
        api_dump["Classes"].sort(key=lambda c: c["Name"])
        
        # Enums
        for enum_name, enum_obj in sorted(self.enums.items(), key=lambda x: x[0]):
            api_dump["Enums"].append({
                "Name": enum_name,
                "Description": enum_obj.get("Description", ""),
                "Examples": enum_obj.get("Examples", []),
                "Items": enum_obj.get("Items", [])
            })
        
        # DataTypes
        for dt in sorted(self.datatypes.values(), key=lambda d: d.name):
            dt_dict = {
                "Name": dt.name,
                "Description": dt.description,
                "Examples": dt.examples,
                "SourceFile": dt.source_file,
                "Members": []
            }
            for member in dt.members:
                member_dict = {
                    "MemberType": member.member_type,
                    "Name": member.name,
                    "Description": member.description,
                    "Examples": member.examples,
                    "Deprecated": member.deprecated,
                    "Internal": member.internal,
                    "InheritedFrom": member.inherited_from
                }
                if member.member_type == "Property":
                    member_dict["ValueType"] = member.value_type
                    member_dict["Default"] = member.default_value
                    member_dict["ReadOnly"] = member.readonly
                elif member.member_type == "Method":
                    member_dict["ReturnType"] = member.return_type
                    member_dict["Parameters"] = [
                        {"Name": p.name, "Type": p.type, "Description": p.description}
                        for p in member.parameters
                    ]
                elif member.member_type == "Event":
                    member_dict["Parameters"] = [
                        {"Name": p.name, "Type": p.type, "Description": p.description}
                        for p in member.parameters
                    ]
                dt_dict["Members"].append(member_dict)
            api_dump["DataTypes"].append(dt_dict)
        
        with open(self.output_file, 'w', encoding='utf-8') as f:
            json.dump(api_dump, f, indent=2)
        
        print(f"\n{'=' * 70}")
        print(f"Extracted {len(self.classes)} classes")
        print(f"API dump written to: {self.output_file}")
        print(f"\nNext: python doc_tool.py generate")
    
    def inherit_parent_docs(self):
        for class_name, class_info in self.classes.items():
            if not class_info.superclass:
                continue
            
            parent = self.classes.get(class_info.superclass)
            if not parent:
                continue
            
            existing_members = {m.name for m in class_info.members}
            
            for parent_member in parent.members:
                if parent_member.name not in existing_members:
                    inherited = Member(
                        member_type=parent_member.member_type,
                        name=parent_member.name,
                        description=parent_member.description,
                        examples=parent_member.examples.copy(),
                        value_type=parent_member.value_type,
                        default_value=parent_member.default_value,
                        readonly=parent_member.readonly,
                        return_type=parent_member.return_type,
                        parameters=parent_member.parameters.copy(),
                        deprecated=parent_member.deprecated,
                        internal=parent_member.internal,
                        inherited_from=parent_member.inherited_from or class_info.superclass
                    )
                    class_info.members.append(inherited)
                    print(f"  {class_name}.{parent_member.name} inherited from {class_info.superclass}")
    
    def process_file(self, file_path: Path):
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        class_pattern = r'/\*\*(.*?)\*/\s*struct\s+(\w+)(?:\s*:\s*public\s+(\w+))?'
        
        for match in re.finditer(class_pattern, content, re.DOTALL):
            doc_block = match.group(1)
            class_name = match.group(2)
            parent_name = match.group(3) if match.group(3) else ""
            
            class_info = self.parse_class_doc(doc_block, class_name, parent_name)
            class_info.source_file = str(file_path.relative_to(self.src_dir))
            
            class_start = match.end()
            class_body = self.extract_class_body(content, class_start)
            
            self.extract_members(class_body, class_info)
            
            # Route structs in datatypes to DataTypes
            if 'datatypes' in str(file_path.as_posix()):
                self.datatypes[class_name] = class_info
                print(f"  └─ [DataType] {class_name}")
            else:
                self.classes[class_name] = class_info
                print(f"  └─ {class_name}" + (f" : {parent_name}" if parent_name else ""))

    def process_enums(self, file_path: Path):
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        enum_pattern = r'(?:/\*\*(?P<doc>.*?)\*/\s*)?enum\s+(?:class\s+)?(?P<name>\w+)\s*(?::\s*\w+)?\s*\{(?P<body>[^}]*)\};'
        for match in re.finditer(enum_pattern, content, re.DOTALL):
            name = match.group('name')
            body = match.group('body') or ""
            doc_text = match.group('doc') or ""
            enum_desc, enum_examples, item_docs = self.parse_enum_doc(doc_text)
            items = []
            value_counter = 0
            for raw in body.split(','):
                line = raw.strip()
                if not line:
                    continue
                trailing_desc = ""
                if '//' in line:
                    line, trailing = line.split('//', 1)
                    trailing_desc = trailing.strip()
                line = re.split(r'/\*', line)[0].strip()
                if not line:
                    continue
                if '=' in line:
                    parts = line.split('=')
                    item_name = parts[0].strip()
                    item_value = parts[1].strip()
                    try:
                        value_counter = int(item_value, 0) + 1
                    except Exception:
                        value_counter += 1
                else:
                    item_name = line.strip()
                    item_value = str(value_counter)
                    value_counter += 1
                desc = item_docs.get(item_name, trailing_desc)
                items.append({"Name": item_name, "Value": item_value, "Description": desc})
            if items:
                self.enums[name] = {
                    "Description": enum_desc,
                    "Examples": enum_examples,
                    "Items": items
                }

    def parse_enum_doc(self, doc_text: str):
        description = ""
        examples: List[str] = []
        item_docs: Dict[str, str] = {}
        if not doc_text:
            return description, examples, item_docs
        lines = doc_text.split('\n')
        current_tag = None
        current_content = []
        def flush(tag, content):
            nonlocal description
            content = content.strip()
            if tag in ("description", "brief"):
                description = content
            elif tag == "example":
                code_match = re.search(r'```lua\s*(.*?)```', content, re.DOTALL)
                if code_match:
                    examples.append(code_match.group(1).strip())
            elif tag == "item":
                m = re.match(r'(\w+)\s*-?\s*(.*)', content)
                if m:
                    item_docs[m.group(1)] = m.group(2).strip()
        for line in lines:
            line = line.strip().lstrip('*').strip()
            if line.startswith('@'):
                if current_tag:
                    flush(current_tag, '\n'.join(current_content))
                parts = line.split(None, 1)
                current_tag = parts[0][1:]
                current_content = [parts[1] if len(parts) > 1 else ""]
            elif current_tag:
                current_content.append(line)
            elif line and not current_tag:
                description += line + " "
        if current_tag:
            flush(current_tag, '\n'.join(current_content))
        description = description.strip()
        return description, examples, item_docs

    def process_datatypes(self, file_path: Path):
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        # Find plain struct definitions without requiring a docblock
        for match in re.finditer(r'\bstruct\s+(\w+)\s*\{', content):
            name = match.group(1)
            # Skip if already captured as a class
            if name in self.classes:
                continue
            # Extract body
            start = match.end() - 1
            body = self.extract_class_body(content, start)
            ci = ClassInfo(name=name)
            ci.source_file = str(file_path.relative_to(self.src_dir))
            if body:
                self.extract_members(body, ci)
            self.datatypes[name] = ci
    
    def parse_class_doc(self, doc_text: str, class_name: str, parent_name: str) -> ClassInfo:
        class_info = ClassInfo(name=class_name, superclass=parent_name)
        
        lines = doc_text.split('\n')
        current_tag = None
        current_content = []
        
        for line in lines:
            line = line.strip().lstrip('*').strip()
            
            if line.startswith('@'):
                if current_tag:
                    self.process_class_tag(class_info, current_tag, '\n'.join(current_content))
                
                parts = line.split(None, 1)
                current_tag = parts[0][1:]
                current_content = [parts[1] if len(parts) > 1 else ""]
            elif current_tag:
                current_content.append(line)
            elif line and not current_tag:
                class_info.description += line + " "
        
        if current_tag:
            self.process_class_tag(class_info, current_tag, '\n'.join(current_content))
        
        class_info.description = class_info.description.strip()
        return class_info
    
    def process_class_tag(self, class_info: ClassInfo, tag: str, content: str):
        content = content.strip()
        
        if tag == "description" or tag == "brief":
            class_info.description = content
        elif tag == "example":
            code_match = re.search(r'```lua\s*(.*?)```', content, re.DOTALL)
            if code_match:
                class_info.examples.append(code_match.group(1).strip())
        elif tag == "category":
            class_info.tags.append(content)
    
    def extract_class_body(self, content: str, start_pos: int) -> str:
        brace_count = 0
        in_class = False
        body_start = -1
        
        for i in range(start_pos, len(content)):
            if content[i] == '{':
                if not in_class:
                    in_class = True
                    body_start = i
                brace_count += 1
            elif content[i] == '}':
                brace_count -= 1
                if brace_count == 0 and in_class:
                    return content[body_start:i+1]
        
        return ""
    
    def extract_members(self, class_body: str, class_info: ClassInfo):
        member_pattern = r'/\*\*(.*?)\*/\s*([^/]+?)(?:;|\{)'
        
        for match in re.finditer(member_pattern, class_body, re.DOTALL):
            doc_block = match.group(1)
            member_decl = match.group(2).strip()
            
            member_info = self.parse_member_doc(doc_block, member_decl)
            
            if member_info:
                class_info.members.append(member_info)
    
    def parse_member_doc(self, doc_text: str, member_decl: str) -> Optional[Member]:
        if 'Signal' in member_decl:
            member_type = "Event"
        elif '(' in member_decl and ')' in member_decl:
            member_type = "Method"
        else:
            member_type = "Property"
        
        member = Member(member_type=member_type, name="")
        
        lines = doc_text.split('\n')
        current_tag = None
        current_content = []
        
        for line in lines:
            line = line.strip().lstrip('*').strip()
            
            if line.startswith('@'):
                if current_tag:
                    self.process_member_tag(member, current_tag, '\n'.join(current_content), member_decl)
                
                parts = line.split(None, 1)
                current_tag = parts[0][1:]
                current_content = [parts[1] if len(parts) > 1 else ""]
            elif current_tag:
                current_content.append(line)
            elif line and not current_tag:
                member.description += line + " "
        
        if current_tag:
            self.process_member_tag(member, current_tag, '\n'.join(current_content), member_decl)
        
        if not member.name:
            member.name = self.extract_member_name(member_decl)
        
        if member.member_type == "Property":
            self.extract_property_info(member, member_decl)
        elif member.member_type == "Method":
            self.extract_method_info(member, member_decl)
        
        # Filter out internal parameters
        member.parameters = [p for p in member.parameters if p.type not in self.INTERNAL_PARAM_TYPES]
        
        member.description = member.description.strip()
        
        return member if member.name else None
    
    def process_member_tag(self, member: Member, tag: str, content: str, decl: str):
        content = content.strip()
        
        if tag == "property" or tag == "event" or tag == "method":
            member.name = content
        elif tag == "description":
            member.description = content
        elif tag == "type":
            member.value_type = content
        elif tag == "returns":
            member.return_type = content
        elif tag == "param":
            param_match = re.match(r'(\w+)\s+(\w+)\s*-?\s*(.*)', content)
            if param_match:
                param_name = param_match.group(1)
                param_type = param_match.group(2)
                
                # Skip internal parameter types
                if param_type in self.INTERNAL_PARAM_TYPES:
                    return
                
                param = Parameter(
                    name=param_name,
                    type=param_type,
                    description=param_match.group(3)
                )
                member.parameters.append(param)
        elif tag == "default":
            member.default_value = content
        elif tag == "readonly":
            member.readonly = True
        elif tag == "deprecated":
            member.deprecated = True
        elif tag == "internal":
            member.internal = True
        elif tag == "example":
            code_match = re.search(r'```lua\s*(.*?)```', content, re.DOTALL)
            if code_match:
                member.examples.append(code_match.group(1).strip())
    
    def extract_member_name(self, decl: str) -> str:
        method_match = re.search(r'\b(\w+)\s*\(', decl)
        if method_match:
            return method_match.group(1)
        
        prop_match = re.search(r'\b(\w+)\s*(?:=|;)', decl)
        if prop_match:
            words = decl.split()
            for i in range(len(words)-1, -1, -1):
                word = words[i].strip('=;')
                if word and not word.startswith('//'):
                    return word
        
        return ""
    
    def extract_property_info(self, member: Member, decl: str):
        match = re.match(r'(\w+(?:<[^>]+>)?)\s+(\w+)(?:\s*=\s*([^;]+))?', decl.strip())
        if match:
            if not member.value_type:
                member.value_type = self.normalize_type(match.group(1))
            if not member.default_value and match.group(3):
                member.default_value = match.group(3).strip()
    
    def extract_method_info(self, member: Member, decl: str):
        match = re.match(r'(\w+)\s+(\w+)\s*\(([^)]*)\)', decl.strip())
        if match:
            if not member.return_type or member.return_type == "void":
                member.return_type = self.normalize_type(match.group(1))
            
            if not member.parameters and match.group(3).strip():
                params_str = match.group(3)
                for param_decl in params_str.split(','):
                    param_decl = param_decl.strip()
                    parts = param_decl.split()
                    if len(parts) >= 2:
                        param_type = self.normalize_type(' '.join(parts[:-1]))
                        
                        # Skip internal parameter types
                        if param_type in self.INTERNAL_PARAM_TYPES:
                            continue
                        
                        param = Parameter(
                            name=parts[-1].strip('&*'),
                            type=param_type
                        )
                        member.parameters.append(param)
    
    def normalize_type(self, type_str: str) -> str:
        type_map = {
            'Vector3Game': 'Vector3',
            'std::string': 'string',
            'double': 'number',
            'float': 'number',
            'int': 'number',
            'bool': 'bool'
        }
        
        for cpp_type, simple_type in type_map.items():
            if cpp_type in type_str:
                return simple_type
        
        return type_str.strip()


class HTMLGenerator:
    def __init__(self, json_file: str, output_dir: str, templates_dir: str = "scripts/documentation/templates"):
        self.json_file = json_file
        self.output_dir = Path(output_dir)
        self.templates_dir = Path(templates_dir)
        self.api_data = {}
        self.class_hierarchy = {}
    
    def generate(self):
        with open(self.json_file, 'r', encoding='utf-8') as f:
            self.api_data = json.load(f)
        
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        self.copy_static_files()
        self.build_class_hierarchy()
        self.build_type_sets()
        
        print(f"\nGenerating documentation for {len(self.api_data['Classes'])} classes, {len(self.api_data.get('Enums', []))} enums, {len(self.api_data.get('DataTypes', []))} datatypes...")
        
        self.generate_index()
        
        for cls in self.api_data["Classes"]:
            self.generate_class_page(cls)
            print(f"  ✓ {cls['Name']}.html")
        for enum in self.api_data.get("Enums", []):
            self.generate_enum_page(enum)
            print(f"  ✓ {enum['Name']}.html")
        for dt in self.api_data.get("DataTypes", []):
            self.generate_datatype_page(dt)
            print(f"  ✓ {dt['Name']}.html")
        
        print(f"\n{'=' * 70}")
        print(f"Documentation generated!")
        print(f"Location: {self.output_dir.absolute()}")
        print(f"Open: {self.output_dir.absolute()}/index.html")
    
    def build_class_hierarchy(self):
        # Create a map of class name to class data
        class_map = {cls['Name']: cls for cls in self.api_data['Classes']}
        
        # Find root classes (no superclass or superclass not in our set)
        roots = []
        for cls in self.api_data['Classes']:
            if not cls.get('Superclass') or cls['Superclass'] not in class_map:
                roots.append(cls['Name'])
        
        # Build tree recursively
        def build_tree(class_name):
            children = [c['Name'] for c in self.api_data['Classes'] if c.get('Superclass') == class_name]
            return {
                'name': class_name,
                'children': [build_tree(child) for child in sorted(children)]
            }
        
        self.class_hierarchy = [build_tree(root) for root in sorted(roots)]
    
    def generate_class_list_html(self, tree_list, indent=0):
        html = ""
        for node in tree_list:
            if indent == 0:
                html += f'                <li><a href="{node["name"]}.html">{node["name"]}</a>'
            else:
                html += f'<li><a href="{node["name"]}.html">{node["name"]}</a>'
            
            if node['children']:
                html += '\n' + '                    ' * (indent + 1) + '<ul class="nav-sublist">\n'
                html += '                    ' * (indent + 2) + self.generate_class_list_html(node['children'], indent + 1)
                html += '                    ' * (indent + 1) + '</ul>\n' + '                    ' * (indent + 1)
            
            html += '</li>\n'
            if indent == 0:
                html += '                '
        
        return html
    
    def generate_enum_list_html(self, enums: list, indent=0):
        html = ""
        for e in enums:
            if indent == 0:
                html += f'                <li><a href="{e["Name"]}.html">{e["Name"]}</a></li>\n                '
            else:
                html += f'<li><a href="{e["Name"]}.html">{e["Name"]}</a></li>\n'
        return html
    
    def generate_datatype_list_html(self, dts: list, indent=0):
        html = ""
        for d in dts:
            if indent == 0:
                html += f'                <li><a href="{d["Name"]}.html">{d["Name"]}</a></li>\n                '
            else:
                html += f'<li><a href="{d["Name"]}.html">{d["Name"]}</a></li>\n'
        return html
    
    def load_template(self, template_name: str) -> str:
        template_path = self.templates_dir / template_name
        with open(template_path, 'r', encoding='utf-8') as f:
            return f.read()
    
    def copy_static_files(self):
        css_source = self.templates_dir / "style.css"
        css_dest = self.output_dir / "style.css"
        
        if css_source.exists():
            with open(css_source, 'r', encoding='utf-8') as f:
                css_content = f.read()
            with open(css_dest, 'w', encoding='utf-8') as f:
                f.write(css_content)
    
    def generate_index(self):
        template = self.load_template("index.html")
        
        class_list = self.generate_class_list_html(self.class_hierarchy)
        enum_list = self.generate_enum_list_html(self.api_data.get('Enums', [])) if self.api_data.get('Enums') else ""
        datatype_list = self.generate_datatype_list_html(self.api_data.get('DataTypes', [])) if self.api_data.get('DataTypes') else ""
        
        html = template.replace("{{CLASS_LIST}}", class_list)
        html = html.replace("{{ENUM_LIST}}", enum_list)
        html = html.replace("{{DATATYPE_LIST}}", datatype_list)
        
        with open(self.output_dir / "index.html", 'w', encoding='utf-8') as f:
            f.write(html)
    
    def generate_class_page(self, cls: dict):
        template = self.load_template("class.html")
        
        toc = ""
        properties = [m for m in cls.get("Members", []) if m["MemberType"] == "Property" and not m.get("Internal")]
        methods = [m for m in cls.get("Members", []) if m["MemberType"] == "Method" and not m.get("Internal")]
        events = [m for m in cls.get("Members", []) if m["MemberType"] == "Event" and not m.get("Internal")]
        
        if properties:
            toc += '                <li><a href="#properties">Properties</a></li>\n'
        if methods:
            toc += '                <li><a href="#methods">Methods</a></li>\n'
        if events:
            toc += '                <li><a href="#events">Events</a></li>\n'
        
        inheritance = ""
        if cls.get("Superclass"):
            inheritance = f'            <p class="inheritance">Inherits <a href="{cls["Superclass"]}.html">{cls["Superclass"]}</a></p>\n'
        
        examples = ""
        if cls.get("Examples"):
            examples = """        <section class="examples">
            <h2>Examples</h2>
"""
            for example in cls["Examples"]:
                examples += f"""            <pre><code class="language-lua">{self.escape_html(example)}</code></pre>
"""
            examples += """        </section>
"""
        
        properties_section = self.generate_properties_section(properties) if properties else ""
        methods_section = self.generate_methods_section(methods) if methods else ""
        events_section = self.generate_events_section(events) if events else ""
        
        html = template.replace("{{CLASS_NAME}}", cls['Name'])
        html = html.replace("{{TOC}}", toc)
        html = html.replace("{{INHERITANCE}}", inheritance)
        html = html.replace("{{DESCRIPTION}}", cls.get('Description', 'No description available.'))
        html = html.replace("{{EXAMPLES}}", examples)
        html = html.replace("{{PROPERTIES_SECTION}}", properties_section)
        html = html.replace("{{METHODS_SECTION}}", methods_section)
        html = html.replace("{{EVENTS_SECTION}}", events_section)
        
        with open(self.output_dir / f"{cls['Name']}.html", 'w', encoding='utf-8') as f:
            f.write(html)

    def generate_enum_page(self, enum: dict):
        template = self.load_template("enum.html")
        items_html = ""
        for it in enum.get('Items', []):
            desc = f" - {self.escape_html(it.get('Description',''))}" if it.get('Description') else ""
            items_html += f"                <li><code>{it['Name']}</code> = <span class=\"type\">{self.escape_html(it['Value'])}</span>{desc}</li>\n"
        examples_html = ""
        for ex in enum.get('Examples', []):
            examples_html += f"            <pre><code class=\"language-lua\">{self.escape_html(ex)}</code></pre>\n"
        html = template.replace("{{ENUM_NAME}}", enum['Name'])
        html = html.replace("{{DESCRIPTION}}", enum.get('Description', ''))
        html = html.replace("{{ENUM_ITEMS}}", items_html)
        html = html.replace("{{ENUM_EXAMPLES}}", examples_html)
        with open(self.output_dir / f"{enum['Name']}.html", 'w', encoding='utf-8') as f:
            f.write(html)

    def generate_datatype_page(self, dt: dict):
        template = self.load_template("datatype.html")
        properties = [m for m in dt.get("Members", []) if m["MemberType"] == "Property" and not m.get("Internal")]
        methods = [m for m in dt.get("Members", []) if m["MemberType"] == "Method" and not m.get("Internal")]
        properties_section = self.generate_properties_section(properties) if properties else ""
        methods_section = self.generate_methods_section(methods) if methods else ""
        html = template.replace("{{DATATYPE_NAME}}", dt['Name'])
        html = html.replace("{{DESCRIPTION}}", dt.get('Description', ''))
        html = html.replace("{{PROPERTIES_SECTION}}", properties_section)
        html = html.replace("{{METHODS_SECTION}}", methods_section)
        with open(self.output_dir / f"{dt['Name']}.html", 'w', encoding='utf-8') as f:
            f.write(html)
    
    def generate_properties_section(self, properties: list) -> str:
        section = """        <section id="properties" class="api-section">
            <h2>Properties</h2>
            <div class="member-list">
"""
        for prop in properties:
            section += self.generate_property_html(prop)
        section += """            </div>
        </section>
"""
        return section
    
    def generate_methods_section(self, methods: list) -> str:
        section = """        <section id="methods" class="api-section">
            <h2>Methods</h2>
            <div class="member-list">
"""
        for method in methods:
            section += self.generate_method_html(method)
        section += """            </div>
        </section>
"""
        return section
    
    def generate_events_section(self, events: list) -> str:
        section = """        <section id="events" class="api-section">
            <h2>Events</h2>
            <div class="member-list">
"""
        for event in events:
            section += self.generate_event_html(event)
        section += """            </div>
        </section>
"""
        return section
    
    def generate_property_html(self, prop: dict) -> str:
        tags = []
        if prop.get("ReadOnly"):
            tags.append('<span class="tag readonly">Read-only</span>')
        if prop.get("Deprecated"):
            tags.append('<span class="tag deprecated">Deprecated</span>')
        if prop.get("InheritedFrom"):
            tags.append(f'<span class="tag inherited">Inherited from <a href="{prop["InheritedFrom"]}.html">{prop["InheritedFrom"]}</a></span>')
        
        tags_html = ' '.join(tags)

        # this doesn't look good, maybe explore different design choices
        default = f' <span class="default">= {prop.get("Default", "")}</span>' if prop.get("Default") else ''
        
        html = f"""                <div class="member">
                    <div class="member-header">
                        <h3 class="member-name">{prop['Name']}</h3>
                        <span class="member-type">{self.link_type(prop.get('ValueType', 'Variant'))}</span>
                        {tags_html}
                    </div>
                    <p class="member-description">{prop.get('Description', 'No description available.')}</p>
"""
        
        if prop.get("Examples"):
            for example in prop["Examples"]:
                html += f"""                    <pre><code class="language-lua">{self.escape_html(example)}</code></pre>
"""
        
        html += """                </div>
"""
        return html
    
    def generate_method_html(self, method: dict) -> str:
        params = ', '.join([f"{p['Name']}: {self.link_type(p['Type'])}" for p in method.get('Parameters', [])])
        signature = f"{method['Name']}({params})"
        
        tags = []
        if method.get("Deprecated"):
            tags.append('<span class="tag deprecated">Deprecated</span>')
        if method.get("InheritedFrom"):
            tags.append(f'<span class="tag inherited">Inherited from <a href="{method["InheritedFrom"]}.html">{method["InheritedFrom"]}</a></span>')
        
        tags_html = ' '.join(tags)
        
        html = f"""                <div class="member">
                    <div class="member-header">
                        <h3 class="member-name">{signature}</h3>
                        <span class="member-type">{self.link_type(method.get('ReturnType', 'void'))}</span>
                        {tags_html}
                    </div>
                    <p class="member-description">{method.get('Description', 'No description available.')}</p>
"""
        
        if method.get("Parameters"):
            html += """                    <div class="parameters">
                        <h4>Parameters:</h4>
                        <ul>
"""
            for param in method["Parameters"]:
                html += f"""                            <li><code>{param['Name']}</code> <span class="type">{param['Type']}</span>"""
                if param.get('Description'):
                    html += f""" - {param['Description']}"""
                html += """</li>
"""
            html += """                        </ul>
                    </div>
"""
        
        if method.get("Examples"):
            for example in method["Examples"]:
                html += f"""                    <pre><code class="language-lua">{self.escape_html(example)}</code></pre>
"""
        
        html += """                </div>
"""
        return html
    
    def generate_event_html(self, event: dict) -> str:
        params = ', '.join([f"{p['Name']}: {self.link_type(p['Type'])}" for p in event.get('Parameters', [])])
        
        tags = []
        if event.get("Deprecated"):
            tags.append('<span class="tag deprecated">Deprecated</span>')
        if event.get("InheritedFrom"):
            tags.append(f'<span class="tag inherited">Inherited from <a href="{event["InheritedFrom"]}.html">{event["InheritedFrom"]}</a></span>')
        
        tags_html = ' '.join(tags)
        
        html = f"""                <div class="member">
                    <div class="member-header">
                        <h3 class="member-name">{event['Name']}</h3>
                        <span class="member-type">Event({params})</span>
                        {tags_html}
                    </div>
                    <p class="member-description">{event.get('Description', 'No description available.')}</p>
"""
        
        if event.get("Examples"):
            for example in event["Examples"]:
                html += f"""                    <pre><code class="language-lua">{self.escape_html(example)}</code></pre>
"""
        
        html += """                </div>
"""
        return html
    
    def escape_html(self, text: str) -> str:
        return text.replace('&', '&amp;').replace('<', '&lt;').replace('>', '&gt;')

    def build_type_sets(self):
        self.class_names = set([c['Name'] for c in self.api_data.get('Classes', [])])
        self.enum_names = set([e['Name'] for e in self.api_data.get('Enums', [])])
        self.datatype_names = set([d['Name'] for d in self.api_data.get('DataTypes', [])])

    def link_type(self, type_name: str) -> str:
        if not type_name:
            return ''
        simple = type_name.strip()
        # Handle common mapped types
        if simple in self.class_names:
            return f'<a class="type" href="{simple}.html">{simple}</a>'
        if simple in self.enum_names:
            return f'<a class="type" href="{simple}.html">{simple}</a>'
        if simple in self.datatype_names:
            return f'<a class="type" href="{simple}.html">{simple}</a>'
        return f'<span class="type">{self.escape_html(simple)}</span>'


def main():
    parser = argparse.ArgumentParser(description="Generate documentation")
    parser.add_argument("command", choices=["extract", "generate", "all"], help="Command to run")
    parser.add_argument("--output", "-o", default="docs", help="Output directory for generated HTML")
    args = parser.parse_args()

    src_dir = "src"
    json_file = "scripts/documentation/api-dump.json"
    output_dir = args.output
    templates_dir = "scripts/documentation/templates"

    if args.command == "extract":
        extractor = CommentExtractor(src_dir, json_file)
        extractor.extract()

    elif args.command == "generate":
        if not Path(json_file).exists():
            print(f"Error: {json_file} not found!")
            print("Run 'python doc_tool.py extract' first")
            sys.exit(1)
        
        generator = HTMLGenerator(json_file, output_dir, templates_dir)
        generator.generate()

    elif args.command == "all":
        extractor = CommentExtractor(src_dir, json_file)
        extractor.extract()
        print("\n")
        generator = HTMLGenerator(json_file, output_dir, templates_dir)
        generator.generate()

if __name__ == "__main__":
    main()