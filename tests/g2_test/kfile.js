let project = new Project('G2 Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addFile('deployment/**');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal');
project.addFile('../../build/Kong-osx-metal/kong.m');

project.addIncludeDir('../../sources/2d');
project.addFile('../../sources/2d/g2unit.c');

project.addKongDir('shaders');

project.flatten();

resolve(project);
