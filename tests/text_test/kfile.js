let project = new Project('Text Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addKongDir('shaders');
project.setDebugDir('deployment');
project.addIncludeDir('../../build/Kong-osx-metal');
project.addFile('../../build/Kong-osx-metal/kong.m');
project.addFile('../../sources/2d/g2unit.c');
project.addFile('../../sources/2d/fontunit.c');
project.addIncludeDir('../../sources/2d');

project.flatten();
resolve(project);
