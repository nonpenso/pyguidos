import os
import sys
import pypandoc
import shutil

# Force Sphinx/nbsphinx to find the bundled pypandoc binary
os.environ['PATH'] += os.pathsep + os.path.dirname(pypandoc.get_pandoc_path())
sys.path.insert(0, os.path.abspath('..'))

# Automatically sync notebooks from project root into docs/notebooks before building
root_notebooks = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'notebooks'))
docs_notebooks = os.path.abspath(os.path.join(os.path.dirname(__file__), 'notebooks'))

if os.path.exists(root_notebooks):
    if os.path.exists(docs_notebooks):
        shutil.rmtree(docs_notebooks)
    shutil.copytree(root_notebooks, docs_notebooks)

project = "pyGuidos"
copyright = "European Union, 2026, Giovanni Caudullo, Peter Vogt, Pierre Soille"
author = "Giovanni Caudullo, Peter Vogt, Pierre Soille"
release = "2.6.4"

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "nbsphinx",
    "sphinx.ext.viewcode",
    "sphinx.ext.githubpages",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]
html_css_files = ["custom.css"]
html_favicon = "https://forest.jrc.ec.europa.eu/static/forest/images/logos/eu/favicon.ico"
html_sidebars = {"index": []}
html_context = {
    "default_mode": "light",
}

html_theme_options = {
    "logo": {
        "text": "pyGuidos",
        "image_light": "https://forest.jrc.ec.europa.eu/static/forest/images/logos/eu/logo-ec--en.svg",
        "image_dark": "https://forest.jrc.ec.europa.eu/static/forest/images/logos/eu/logo-ec--en.svg",
    },
    "icon_links": [
        {
            "name": "GitLab",
            "url": "https://jrc-forest.pages.code.europa.eu/guidos/pyguidos",
            "icon": "fa-brands fa-gitlab",
        },
    ],
    "show_prev_next": True,
    "navbar_align": "left",    
    "footer_start": ["copyright"],
    "footer_end": ["theme-version"],
    "secondary_sidebar_items": ["page-toc", "edit-this-page"], 
}

nbsphinx_allow_external_files = True
nbsphinx_execute = 'never'
highlight_language = 'python'