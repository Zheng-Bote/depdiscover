import json
import graphviz
import sys
import os
import time

# Konfiguration
SKIP_SYSTEM_LIBS = True  # Setze auf False, wenn du ALLES sehen willst
OUTPUT_FORMAT = 'png'    # 'svg' ist oft schneller und besser zoombar
ENGINE = 'dot'           # 'dot' ist schön, 'sfdp' ist viel schneller bei großen Graphen

def get_security_color(dep):
    cves = dep.get("cves", [])
    if not cves: return "lightgrey"
    
    first_id = cves[0].get("id", "")
    if first_id == "SAFE": return "#90EE90" # Grün
    elif first_id == "NOT-CHECKED": return "lightgrey"
    elif first_id == "CHECK-ERROR": return "#FFE4B5" # Gelb
    else: return "#FF6347" # Rot (Gefahr)

def visualize(json_file):
    print(f"--- Starte Visualisierung für {json_file} ---")
    
    if not os.path.exists(json_file):
        print(f"❌ Fehler: Datei nicht gefunden: {json_file}")
        return

    # 1. JSON Laden
    print("1. Lade JSON...", end="", flush=True)
    try:
        with open(json_file, 'r') as f:
            data = json.load(f)
        print(" Fertig.")
    except Exception as e:
        print(f"\n❌ JSON Fehler: {e}")
        return

    # 2. Graph initialisieren
    dot = graphviz.Digraph(comment='Dependency Graph')
    dot.attr(rankdir='LR') 
    # Globales Design für Performance
    dot.attr('node', shape='box', style='filled', fontname='Helvetica')
    
    project_name = data.get("project_name", "Project")
    dot.node('ROOT', project_name, shape='doubleoctagon', fillcolor='lightblue')

    dependencies = data.get("dependencies", [])
    count = 0
    skipped = 0

    print("2. Baue Graph-Knoten...", end="", flush=True)
    
    for dep in dependencies:
        name = dep.get("name", "unknown")
        dep_type = dep.get("type", "unknown")
        version = dep.get("version", "?")

        # FILTER: System-Libs überspringen?
        if SKIP_SYSTEM_LIBS and dep_type == "system":
            skipped += 1
            continue

        label = f"{name}\n{version}\n({dep_type})"
        color = get_security_color(dep)
        
        # Knoten & Kante
        dot.node(name, label, fillcolor=color)
        dot.edge('ROOT', name)
        count += 1

    print(f" Fertig. ({count} Knoten, {skipped} System-Libs übersprungen)")

    # 3. Source Datei speichern (.gv)
    output_filename = 'dependency_graph'
    print(f"3. Speichere DOT-Source ({output_filename}.gv)...", end="", flush=True)
    
    # save() schreibt nur den Text-Code (geht sofort)
    src_path = dot.save(filename=output_filename + '.gv')
    print(" Fertig.")

    # 4. Rendering (Das kann dauern)
    print(f"4. Rendere Bild ({ENGINE} -> {OUTPUT_FORMAT})... ", end="", flush=True)
    print("(Das kann bei vielen Knoten dauern!)")
    
    try:
        start_time = time.time()
        # Wir ändern die Engine falls es zu viele Knoten sind
        if count > 100 and ENGINE == 'dot':
            print("   ⚠️  Viele Knoten erkannt. Wechsle zu 'sfdp' Engine für Performance...")
            dot.engine = 'sfdp'
        
        dot.render(output_filename, view=False, format=OUTPUT_FORMAT, cleanup=False)
        duration = time.time() - start_time
        print(f"✅ Fertig in {duration:.2f} Sekunden!")
        print(f"   Datei erstellt: {output_filename}.{OUTPUT_FORMAT}")
        
    except graphviz.backend.ExecutableNotFound:
        print("\n❌ Fehler: 'dot' Executable nicht gefunden.")
        print("   Bitte installiere Graphviz (sudo apt install graphviz).")
        print(f"   Die Quelldatei '{src_path}' wurde trotzdem gespeichert.")
    except Exception as e:
        print(f"\n❌ Rendering Fehler: {e}")
        print(f"   Tipp: Du kannst die Datei '{output_filename}.gv' auch online ansehen (z.B. edotor.net).")

if __name__ == "__main__":
    path = "depdiscover.json"
    if len(sys.argv) > 1:
        path = sys.argv[1]
    
    visualize(path)